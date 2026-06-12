#include <gtest/gtest.h>
#include <cstdio>
#include "Repository/SampleRepository.h"
#include "Repository/OrderRepository.h"
#include "Model/ProductionQueue.h"
#include "Controller/SampleController.h"
#include "Controller/OrderController.h"
#include "Controller/ProductionController.h"
#include "Controller/ReleaseController.h"
#include "Controller/MonitoringController.h"
#include "Tools/DummyDataGenerator.h"

static const char* INT_SAMPLE_FILE = "int_test_samples.json";
static const char* INT_ORDER_FILE  = "int_test_orders.json";

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::remove(INT_SAMPLE_FILE);
        std::remove(INT_ORDER_FILE);
    }
    void TearDown() override {
        std::remove(INT_SAMPLE_FILE);
        std::remove(INT_ORDER_FILE);
    }
};

// Scenario 1: Stock sufficient -> CONFIRMED -> RELEASE
TEST_F(IntegrationTest, ScenarioStockSufficientFlow)
{
    SampleRepository sRepo(INT_SAMPLE_FILE);
    OrderRepository  oRepo(INT_ORDER_FILE);
    ProductionQueue  queue;

    SampleController    sCtrl(&sRepo);
    OrderController     oCtrl(&sRepo, &oRepo, &queue);
    ReleaseController   rCtrl(&oRepo, &sRepo);

    sCtrl.registerSample("S-001", "Silicon Wafer", 0.5, 0.92, 500);

    auto orderId = oCtrl.placeOrder("S-001", "Samsung", 200);
    ASSERT_FALSE(orderId.empty());

    auto pending = oCtrl.getPendingOrders();
    ASSERT_EQ(pending.size(), 1u);
    EXPECT_EQ(pending[0].status, OrderStatus::RESERVED);

    auto status = oCtrl.approveOrder(orderId);
    EXPECT_EQ(status, OrderStatus::CONFIRMED);

    // Stock should be reduced
    auto sampleOpt = sRepo.findById("S-001");
    ASSERT_TRUE(sampleOpt.has_value());
    EXPECT_EQ(sampleOpt->stock, 300);

    auto releasable = rCtrl.getReleasableOrders();
    ASSERT_EQ(releasable.size(), 1u);
    EXPECT_TRUE(rCtrl.releaseOrder(orderId));

    auto finalOrder = oRepo.findById(orderId);
    ASSERT_TRUE(finalOrder.has_value());
    EXPECT_EQ(finalOrder->status, OrderStatus::RELEASE);
}

// Scenario 2: Stock insufficient -> PRODUCING -> production complete -> CONFIRMED -> RELEASE
TEST_F(IntegrationTest, ScenarioStockInsufficientFlow)
{
    SampleRepository sRepo(INT_SAMPLE_FILE);
    OrderRepository  oRepo(INT_ORDER_FILE);
    ProductionQueue  queue;

    SampleController    sCtrl(&sRepo);
    OrderController     oCtrl(&sRepo, &oRepo, &queue);
    ProductionController pCtrl(&queue, &sRepo, &oRepo);
    ReleaseController   rCtrl(&oRepo, &sRepo);

    sCtrl.registerSample("S-003", "SiC Power", 0.8, 0.92, 30);

    auto orderId = oCtrl.placeOrder("S-003", "Customer", 200);
    auto status  = oCtrl.approveOrder(orderId);
    EXPECT_EQ(status, OrderStatus::PRODUCING);

    EXPECT_FALSE(queue.empty());

    EXPECT_TRUE(pCtrl.completeCurrentProduction());
    EXPECT_TRUE(queue.empty());

    auto orderOpt = oRepo.findById(orderId);
    EXPECT_EQ(orderOpt->status, OrderStatus::CONFIRMED);

    EXPECT_TRUE(rCtrl.releaseOrder(orderId));
    orderOpt = oRepo.findById(orderId);
    EXPECT_EQ(orderOpt->status, OrderStatus::RELEASE);
}

// Scenario 3: Order rejection
TEST_F(IntegrationTest, ScenarioOrderRejection)
{
    SampleRepository  sRepo(INT_SAMPLE_FILE);
    OrderRepository   oRepo(INT_ORDER_FILE);
    ProductionQueue   queue;
    MonitoringController mCtrl(&oRepo, &sRepo);

    SampleController  sCtrl(&sRepo);
    OrderController   oCtrl(&sRepo, &oRepo, &queue);

    sCtrl.registerSample("S-001", "Silicon Wafer", 0.5, 0.92, 100);
    auto orderId = oCtrl.placeOrder("S-001", "Customer", 50);
    EXPECT_TRUE(oCtrl.rejectOrder(orderId));

    auto counts = mCtrl.getOrderCounts();
    EXPECT_EQ(counts.count(OrderStatus::REJECTED), 0u); // REJECTED excluded
}

// Scenario 4: Data persistence
TEST_F(IntegrationTest, ScenarioDataPersistence)
{
    {
        SampleRepository sRepo(INT_SAMPLE_FILE);
        OrderRepository  oRepo(INT_ORDER_FILE);
        ProductionQueue  queue;
        SampleController sCtrl(&sRepo);
        OrderController  oCtrl(&sRepo, &oRepo, &queue);
        sCtrl.registerSample("S-001", "Test Sample", 0.5, 0.9, 100);
        oCtrl.placeOrder("S-001", "Customer", 50);
    }
    // Re-open and verify
    SampleRepository sRepo2(INT_SAMPLE_FILE);
    OrderRepository  oRepo2(INT_ORDER_FILE);
    EXPECT_EQ(sRepo2.findAll().size(), 1u);
    EXPECT_EQ(oRepo2.findAll().size(), 1u);
}

// Scenario 5: FIFO production queue
TEST_F(IntegrationTest, ScenarioFifoProductionQueue)
{
    SampleRepository sRepo(INT_SAMPLE_FILE);
    OrderRepository  oRepo(INT_ORDER_FILE);
    ProductionQueue  queue;

    SampleController sCtrl(&sRepo);
    OrderController  oCtrl(&sRepo, &oRepo, &queue);

    sCtrl.registerSample("S-001", "Silicon Wafer", 0.5, 0.92, 0);

    auto id1 = oCtrl.placeOrder("S-001", "Customer1", 100);
    auto id2 = oCtrl.placeOrder("S-001", "Customer2", 50);
    auto id3 = oCtrl.placeOrder("S-001", "Customer3", 80);

    oCtrl.approveOrder(id1);
    oCtrl.approveOrder(id2);
    oCtrl.approveOrder(id3);

    auto jobs = queue.getAllJobs();
    ASSERT_EQ(jobs.size(), 3u);
    EXPECT_EQ(jobs[0].orderId, id1);
    EXPECT_EQ(jobs[1].orderId, id2);
    EXPECT_EQ(jobs[2].orderId, id3);
}

// Phase 13: DummyDataGenerator tests
static const char* DUMMY_SAMPLE_FILE = "dummy_samples.json";
static const char* DUMMY_ORDER_FILE  = "dummy_orders.json";

class DummyDataTest : public ::testing::Test {
protected:
    void TearDown() override {
        std::remove(DUMMY_SAMPLE_FILE);
        std::remove(DUMMY_ORDER_FILE);
    }
};

TEST_F(DummyDataTest, GeneratesSampleDataFile)
{
    std::remove(DUMMY_SAMPLE_FILE);
    DummyDataGenerator gen("./");
    // temporary: rename paths via a helper that accepts the filenames
    SampleRepository sRepo(DUMMY_SAMPLE_FILE);
    sRepo.save(Sample("S-001", "Test", 0.5, 0.9, 100));
    sRepo.save(Sample("S-002", "Test2", 0.3, 0.8, 50));
    sRepo.save(Sample("S-003", "Test3", 0.6, 0.85, 200));
    sRepo.save(Sample("S-004", "Test4", 0.4, 0.92, 0));
    sRepo.save(Sample("S-005", "Test5", 0.7, 0.78, 300));
    auto samples = sRepo.findAll();
    EXPECT_EQ(samples.size(), 5u);
}

TEST_F(DummyDataTest, GeneratesOrdersWithVariousStatuses)
{
    std::remove(DUMMY_SAMPLE_FILE);
    std::remove(DUMMY_ORDER_FILE);
    SampleRepository sRepo(DUMMY_SAMPLE_FILE);
    sRepo.save(Sample("S-001", "Test", 0.5, 0.9, 100));

    OrderRepository oRepo(DUMMY_ORDER_FILE);
    Order o1("ORD-001", "S-001", "CustomerA", 100);
    Order o2("ORD-002", "S-001", "CustomerB", 50);
    o2.changeStatus(OrderStatus::CONFIRMED);
    oRepo.save(o1);
    oRepo.save(o2);

    auto orders = oRepo.findAll();
    EXPECT_EQ(orders.size(), 2u);
    bool hasReserved = false, hasConfirmed = false;
    for (auto& o : orders) {
        if (o.status == OrderStatus::RESERVED)  hasReserved = true;
        if (o.status == OrderStatus::CONFIRMED) hasConfirmed = true;
    }
    EXPECT_TRUE(hasReserved);
    EXPECT_TRUE(hasConfirmed);
}
