#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/Order.h"
#include "Controller/MonitoringController.h"
#include "Controller/ReleaseController.h"

using ::testing::Return;
using ::testing::_;

class MockSampleRepo4 : public IRepository<Sample> {
public:
    MOCK_METHOD(void, save, (const Sample&), (override));
    MOCK_METHOD(std::optional<Sample>, findById, (const std::string&), (override));
    MOCK_METHOD(std::vector<Sample>, findAll, (), (override));
    MOCK_METHOD(void, update, (const Sample&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};

class MockOrderRepo4 : public IRepository<Order> {
public:
    MOCK_METHOD(void, save, (const Order&), (override));
    MOCK_METHOD(std::optional<Order>, findById, (const std::string&), (override));
    MOCK_METHOD(std::vector<Order>, findAll, (), (override));
    MOCK_METHOD(void, update, (const Order&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};

// Helper to create an order with a specific status
static Order makeOrder(const std::string& id, OrderStatus status) {
    Order o(id, "S-001", "Customer", 100);
    if (status == OrderStatus::CONFIRMED) {
        o.changeStatus(OrderStatus::CONFIRMED);
    } else if (status == OrderStatus::PRODUCING) {
        o.changeStatus(OrderStatus::PRODUCING);
    } else if (status == OrderStatus::REJECTED) {
        o.changeStatus(OrderStatus::REJECTED);
    } else if (status == OrderStatus::RELEASE) {
        o.changeStatus(OrderStatus::CONFIRMED);
        o.changeStatus(OrderStatus::RELEASE);
    }
    return o;
}

// ========== MonitoringController Tests ==========

// Step 9-1: Count orders by status, REJECTED excluded
TEST(MonitoringControllerTest, CountOrdersByStatusExcludesRejected)
{
    MockOrderRepo4 mockOrderRepo;
    MockSampleRepo4 mockSampleRepo;
    MonitoringController controller(&mockOrderRepo, &mockSampleRepo);

    std::vector<Order> orders = {
        makeOrder("ORD-001", OrderStatus::RESERVED),
        makeOrder("ORD-002", OrderStatus::CONFIRMED),
        makeOrder("ORD-003", OrderStatus::CONFIRMED),
        makeOrder("ORD-004", OrderStatus::REJECTED),
        makeOrder("ORD-005", OrderStatus::PRODUCING),
        makeOrder("ORD-006", OrderStatus::RELEASE),
    };
    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(orders));

    auto counts = controller.getOrderCounts();
    EXPECT_EQ(counts[OrderStatus::RESERVED], 1);
    EXPECT_EQ(counts[OrderStatus::CONFIRMED], 2);
    EXPECT_EQ(counts[OrderStatus::PRODUCING], 1);
    EXPECT_EQ(counts[OrderStatus::RELEASE], 1);
    EXPECT_EQ(counts.count(OrderStatus::REJECTED), 0u);
}

// Step 9-2: Stock DEPLETED when stock == 0
TEST(MonitoringControllerTest, StockStatusIsDepletedWhenZero)
{
    MockOrderRepo4 mockOrderRepo;
    MockSampleRepo4 mockSampleRepo;
    MonitoringController controller(&mockOrderRepo, &mockSampleRepo);

    Sample s("S-005", "Oxide Wafer", 0.6, 0.88, 0);
    EXPECT_CALL(mockSampleRepo, findAll()).WillOnce(Return(std::vector<Sample>{s}));
    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(std::vector<Order>{}));

    auto statuses = controller.getStockStatuses();
    EXPECT_EQ(statuses["S-005"], StockStatus::DEPLETED);
}

// Step 9-3: Stock SHORTAGE when stock < pending orders
TEST(MonitoringControllerTest, StockStatusIsShortageWhenLessThanPendingOrders)
{
    MockOrderRepo4 mockOrderRepo;
    MockSampleRepo4 mockSampleRepo;
    MonitoringController controller(&mockOrderRepo, &mockSampleRepo);

    Sample s("S-003", "SiC", 0.8, 0.92, 30);
    Order o("ORD-001", "S-003", "Customer", 200);
    o.changeStatus(OrderStatus::CONFIRMED);

    EXPECT_CALL(mockSampleRepo, findAll()).WillOnce(Return(std::vector<Sample>{s}));
    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(std::vector<Order>{o}));

    auto statuses = controller.getStockStatuses();
    EXPECT_EQ(statuses["S-003"], StockStatus::SHORTAGE);
}

// Step 9-4: Stock SUFFICIENT when stock >= pending orders
TEST(MonitoringControllerTest, StockStatusIsSufficientWhenStockCoversOrders)
{
    MockOrderRepo4 mockOrderRepo;
    MockSampleRepo4 mockSampleRepo;
    MonitoringController controller(&mockOrderRepo, &mockSampleRepo);

    Sample s("S-001", "Silicon Wafer", 0.5, 0.92, 480);
    Order o("ORD-001", "S-001", "Customer", 100);
    o.changeStatus(OrderStatus::CONFIRMED);

    EXPECT_CALL(mockSampleRepo, findAll()).WillOnce(Return(std::vector<Sample>{s}));
    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(std::vector<Order>{o}));

    auto statuses = controller.getStockStatuses();
    EXPECT_EQ(statuses["S-001"], StockStatus::SUFFICIENT);
}

// ========== ReleaseController Tests ==========

// Step 10-1: Get releasable orders (CONFIRMED only)
TEST(ReleaseControllerTest, GetReleasableOrdersReturnsOnlyConfirmed)
{
    MockOrderRepo4 mockOrderRepo;
    MockSampleRepo4 mockSampleRepo;
    ReleaseController controller(&mockOrderRepo, &mockSampleRepo);

    std::vector<Order> all = {
        makeOrder("ORD-001", OrderStatus::CONFIRMED),
        makeOrder("ORD-002", OrderStatus::PRODUCING),
        makeOrder("ORD-003", OrderStatus::CONFIRMED),
    };
    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(all));

    auto releasable = controller.getReleasableOrders();
    EXPECT_EQ(releasable.size(), 2u);
}

// Step 10-2: Release order -> RELEASE
TEST(ReleaseControllerTest, ReleaseOrderChangesStatusToRelease)
{
    MockOrderRepo4 mockOrderRepo;
    MockSampleRepo4 mockSampleRepo;
    ReleaseController controller(&mockOrderRepo, &mockSampleRepo);

    Order order("ORD-001", "S-001", "Customer", 150);
    order.changeStatus(OrderStatus::CONFIRMED);

    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockOrderRepo, update(_)).Times(1);

    bool result = controller.releaseOrder("ORD-001");
    EXPECT_TRUE(result);
}

// Step 10-3: Cannot release non-CONFIRMED order
TEST(ReleaseControllerTest, CannotReleaseOrderThatIsNotConfirmed)
{
    MockOrderRepo4 mockOrderRepo;
    MockSampleRepo4 mockSampleRepo;
    ReleaseController controller(&mockOrderRepo, &mockSampleRepo);

    Order order("ORD-001", "S-001", "Customer", 150); // RESERVED
    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockOrderRepo, update(_)).Times(0);

    bool result = controller.releaseOrder("ORD-001");
    EXPECT_FALSE(result);
}
