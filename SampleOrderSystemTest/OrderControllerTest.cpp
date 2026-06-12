#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/Order.h"
#include "Model/IProductionQueue.h"
#include "Model/ProductionJob.h"
#include "Controller/OrderController.h"

using ::testing::Return;
using ::testing::_;

// Reuse MockSampleRepository defined here (separate TU from SampleControllerTest)
class MockSampleRepo2 : public IRepository<Sample> {
public:
    MOCK_METHOD(void, save, (const Sample&), (override));
    MOCK_METHOD(std::optional<Sample>, findById, (const std::string&), (override));
    MOCK_METHOD(std::vector<Sample>, findAll, (), (override));
    MOCK_METHOD(void, update, (const Sample&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};

class MockOrderRepo : public IRepository<Order> {
public:
    MOCK_METHOD(void, save, (const Order&), (override));
    MOCK_METHOD(std::optional<Order>, findById, (const std::string&), (override));
    MOCK_METHOD(std::vector<Order>, findAll, (), (override));
    MOCK_METHOD(void, update, (const Order&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};

class MockProdQueue : public IProductionQueue {
public:
    MOCK_METHOD(void, enqueue, (const ProductionJob&), (override));
    MOCK_METHOD(std::optional<ProductionJob>, front, (), (const, override));
    MOCK_METHOD(void, dequeue, (), (override));
    MOCK_METHOD(std::vector<ProductionJob>, getAllJobs, (), (const, override));
    MOCK_METHOD(bool, empty, (), (const, override));
};

// Step 6-2: Place order creates RESERVED order
TEST(OrderControllerTest, PlaceOrderCreatesReservedOrder)
{
    MockSampleRepo2 mockSampleRepo;
    MockOrderRepo mockOrderRepo;
    OrderController controller(&mockSampleRepo, &mockOrderRepo);

    Sample s("S-001", "Silicon Wafer", 0.5, 0.92, 480);
    EXPECT_CALL(mockSampleRepo, findById("S-001")).WillOnce(Return(s));
    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(std::vector<Order>{}));
    EXPECT_CALL(mockOrderRepo, save(_)).Times(1);

    auto orderId = controller.placeOrder("S-001", "Samsung", 200);
    EXPECT_FALSE(orderId.empty());
    EXPECT_THAT(orderId, ::testing::StartsWith("ORD-"));
}

// Step 6-3: Reject order for unregistered sample
TEST(OrderControllerTest, RejectsOrderForUnregisteredSample)
{
    MockSampleRepo2 mockSampleRepo;
    MockOrderRepo mockOrderRepo;
    OrderController controller(&mockSampleRepo, &mockOrderRepo);

    EXPECT_CALL(mockSampleRepo, findById("S-999")).WillOnce(Return(std::nullopt));
    EXPECT_CALL(mockOrderRepo, save(_)).Times(0);

    auto orderId = controller.placeOrder("S-999", "Customer", 100);
    EXPECT_TRUE(orderId.empty());
}

// Step 7-2: Stock sufficient -> CONFIRMED
TEST(OrderControllerTest, ApproveOrderChangesToConfirmedWhenStockSufficient)
{
    MockSampleRepo2 mockSampleRepo;
    MockOrderRepo mockOrderRepo;
    MockProdQueue mockQueue;
    OrderController controller(&mockSampleRepo, &mockOrderRepo, &mockQueue);

    Order order("ORD-001", "S-001", "Customer", 100);
    Sample sample("S-001", "Sample", 0.5, 0.92, 200);

    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockSampleRepo, findById("S-001")).WillOnce(Return(sample));
    EXPECT_CALL(mockOrderRepo, update(_)).Times(1);
    EXPECT_CALL(mockSampleRepo, update(_)).Times(1);
    EXPECT_CALL(mockQueue, enqueue(_)).Times(0);

    auto result = controller.approveOrder("ORD-001");
    EXPECT_EQ(result, OrderStatus::CONFIRMED);
}

// Step 7-3: Stock insufficient -> PRODUCING + enqueue
TEST(OrderControllerTest, ApproveOrderChangesToProducingWhenStockInsufficient)
{
    MockSampleRepo2 mockSampleRepo;
    MockOrderRepo mockOrderRepo;
    MockProdQueue mockQueue;
    OrderController controller(&mockSampleRepo, &mockOrderRepo, &mockQueue);

    Order order("ORD-001", "S-001", "Customer", 200);
    Sample sample("S-001", "SiC", 0.8, 0.92, 30);

    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockSampleRepo, findById("S-001")).WillOnce(Return(sample));
    EXPECT_CALL(mockOrderRepo, update(_)).Times(1);
    EXPECT_CALL(mockQueue, enqueue(_)).Times(1);

    auto result = controller.approveOrder("ORD-001");
    EXPECT_EQ(result, OrderStatus::PRODUCING);
}

// Step 7-4: Reject order -> REJECTED
TEST(OrderControllerTest, RejectOrderChangesToRejected)
{
    MockSampleRepo2 mockSampleRepo;
    MockOrderRepo mockOrderRepo;
    MockProdQueue mockQueue;
    OrderController controller(&mockSampleRepo, &mockOrderRepo, &mockQueue);

    Order order("ORD-001", "S-001", "Customer", 100);
    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockOrderRepo, update(_)).Times(1);

    bool result = controller.rejectOrder("ORD-001");
    EXPECT_TRUE(result);
}

// Step 7-5: Get pending orders (RESERVED only)
TEST(OrderControllerTest, GetPendingOrdersReturnsOnlyReserved)
{
    MockSampleRepo2 mockSampleRepo;
    MockOrderRepo mockOrderRepo;
    MockProdQueue mockQueue;
    OrderController controller(&mockSampleRepo, &mockOrderRepo, &mockQueue);

    Order o1("ORD-001", "S-001", "CustomerA", 100);
    Order o2("ORD-002", "S-001", "CustomerB", 50);
    o2.changeStatus(OrderStatus::CONFIRMED);
    std::vector<Order> allOrders = {o1, o2};

    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(allOrders));

    auto pending = controller.getPendingOrders();
    EXPECT_EQ(pending.size(), 1u);
    EXPECT_EQ(pending[0].orderId, "ORD-001");
}
