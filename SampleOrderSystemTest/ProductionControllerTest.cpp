#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/Order.h"
#include "Model/ProductionJob.h"
#include "Model/IProductionQueue.h"
#include "Model/ProductionQueue.h"
#include "Controller/ProductionController.h"

using ::testing::Return;
using ::testing::_;

class MockSampleRepo3 : public IRepository<Sample> {
public:
    MOCK_METHOD(void, save, (const Sample&), (override));
    MOCK_METHOD(std::optional<Sample>, findById, (const std::string&), (override));
    MOCK_METHOD(std::vector<Sample>, findAll, (), (override));
    MOCK_METHOD(void, update, (const Sample&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};

class MockOrderRepo3 : public IRepository<Order> {
public:
    MOCK_METHOD(void, save, (const Order&), (override));
    MOCK_METHOD(std::optional<Order>, findById, (const std::string&), (override));
    MOCK_METHOD(std::vector<Order>, findAll, (), (override));
    MOCK_METHOD(void, update, (const Order&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};

class MockProdQueue3 : public IProductionQueue {
public:
    MOCK_METHOD(void, enqueue, (const ProductionJob&), (override));
    MOCK_METHOD(std::optional<ProductionJob>, front, (), (const, override));
    MOCK_METHOD(void, dequeue, (), (override));
    MOCK_METHOD(std::vector<ProductionJob>, getAllJobs, (), (const, override));
    MOCK_METHOD(bool, empty, (), (const, override));
};

// Step 8-1: FIFO order guarantee (concrete ProductionQueue)
TEST(ProductionControllerTest, ProductionQueueFollowsFifoOrder)
{
    ProductionQueue queue;
    ProductionJob job1("ORD-001", "S-001", 100, 0.92, 0.5);
    ProductionJob job2("ORD-002", "S-002", 50, 0.78, 0.3);
    queue.enqueue(job1);
    queue.enqueue(job2);
    EXPECT_EQ(queue.front()->orderId, "ORD-001");
    queue.dequeue();
    EXPECT_EQ(queue.front()->orderId, "ORD-002");
}

// Step 8-2: Get current job (front of queue)
TEST(ProductionControllerTest, GetCurrentJobReturnsFirstInQueue)
{
    MockProdQueue3 mockQueue;
    MockSampleRepo3 mockSampleRepo;
    MockOrderRepo3 mockOrderRepo;
    ProductionController controller(&mockQueue, &mockSampleRepo, &mockOrderRepo);

    ProductionJob job("ORD-001", "S-001", 170, 0.92, 0.8); // actualProduction=206
    EXPECT_CALL(mockQueue, front()).WillOnce(Return(job));

    auto current = controller.getCurrentJob();
    ASSERT_TRUE(current.has_value());
    EXPECT_EQ(current->orderId, "ORD-001");
    EXPECT_EQ(current->actualProduction, 206);
}

// Step 8-3: Get waiting jobs (all except first)
TEST(ProductionControllerTest, GetWaitingJobsReturnsQueueExcludingFirst)
{
    MockProdQueue3 mockQueue;
    MockSampleRepo3 mockSampleRepo;
    MockOrderRepo3 mockOrderRepo;
    ProductionController controller(&mockQueue, &mockSampleRepo, &mockOrderRepo);

    std::vector<ProductionJob> all = {
        ProductionJob("ORD-001", "S-001", 100, 0.92, 0.5),
        ProductionJob("ORD-002", "S-002", 50, 0.78, 0.3),
        ProductionJob("ORD-003", "S-003", 80, 0.88, 0.6)
    };
    EXPECT_CALL(mockQueue, getAllJobs()).WillOnce(Return(all));

    auto waiting = controller.getWaitingJobs();
    EXPECT_EQ(waiting.size(), 2u);
}

// Step 8-4: Complete production -> CONFIRMED + stock increase
TEST(ProductionControllerTest, CompleteProductionChangesOrderToConfirmedAndIncreasesStock)
{
    MockProdQueue3 mockQueue;
    MockSampleRepo3 mockSampleRepo;
    MockOrderRepo3 mockOrderRepo;
    ProductionController controller(&mockQueue, &mockSampleRepo, &mockOrderRepo);

    ProductionJob job("ORD-001", "S-001", 170, 0.92, 0.8); // actualProduction=206
    Order order("ORD-001", "S-001", "Customer", 200);
    order.changeStatus(OrderStatus::PRODUCING);
    Sample sample("S-001", "Sample", 0.8, 0.92, 30);

    EXPECT_CALL(mockQueue, front()).WillOnce(Return(job));
    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockSampleRepo, findById("S-001")).WillOnce(Return(sample));
    EXPECT_CALL(mockSampleRepo, update(_)).Times(1);
    EXPECT_CALL(mockOrderRepo, update(_)).Times(1);
    EXPECT_CALL(mockQueue, dequeue()).Times(1);

    bool result = controller.completeCurrentProduction();
    EXPECT_TRUE(result);
}
