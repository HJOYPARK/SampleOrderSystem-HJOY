#include <gtest/gtest.h>
#include <cstdio>
#include "Model/Sample.h"
#include "Model/Order.h"
#include "Repository/JsonSerializer.h"
#include "Repository/SampleRepository.h"
#include "Repository/OrderRepository.h"

// ========== JsonSerializer Tests ==========

// Step 4-2: Sample serialization
TEST(JsonSerializerTest, SerializesSampleToJsonString)
{
    Sample s("S-001", "Silicon Wafer", 0.5, 0.92, 480);
    std::string json = JsonSerializer::toJson(s);
    EXPECT_NE(json.find("\"id\":\"S-001\""), std::string::npos);
    EXPECT_NE(json.find("\"stock\":480"), std::string::npos);
}

// Step 4-3: Sample deserialization
TEST(JsonSerializerTest, DeserializesSampleFromJsonString)
{
    std::string json = R"({"id":"S-001","name":"Silicon Wafer","avgProductionTime":0.5,"yieldRate":0.92,"stock":480})";
    Sample s = JsonSerializer::sampleFromJson(json);
    EXPECT_EQ(s.id, "S-001");
    EXPECT_EQ(s.name, "Silicon Wafer");
    EXPECT_DOUBLE_EQ(s.avgProductionTime, 0.5);
    EXPECT_DOUBLE_EQ(s.yieldRate, 0.92);
    EXPECT_EQ(s.stock, 480);
}

// Step 4-4: Order round-trip
TEST(JsonSerializerTest, SerializesAndDeserializesOrderRoundTrip)
{
    Order original("ORD-20260416-0001", "S-001", "Samsung", 200);
    std::string json = JsonSerializer::toJson(original);
    Order restored = JsonSerializer::orderFromJson(json);
    EXPECT_EQ(restored.orderId, original.orderId);
    EXPECT_EQ(restored.sampleId, original.sampleId);
    EXPECT_EQ(restored.customerName, original.customerName);
    EXPECT_EQ(restored.quantity, original.quantity);
    EXPECT_EQ(restored.status, original.status);
}

TEST(JsonSerializerTest, SerializesOrderStatusAsString)
{
    Order o("ORD-20260416-0001", "S-001", "Customer", 100);
    o.changeStatus(OrderStatus::CONFIRMED);
    std::string json = JsonSerializer::toJson(o);
    EXPECT_NE(json.find("\"CONFIRMED\""), std::string::npos);
}

// ========== SampleRepository Tests ==========

static const char* SAMPLE_TEST_FILE = "test_samples_tmp.json";

class SampleRepositoryTest : public ::testing::Test {
protected:
    void TearDown() override {
        std::remove(SAMPLE_TEST_FILE);
    }
};

TEST_F(SampleRepositoryTest, SaveAndFindSampleById)
{
    SampleRepository repo(SAMPLE_TEST_FILE);
    Sample s("S-001", "Test Sample", 0.5, 0.9, 100);
    repo.save(s);
    auto found = repo.findById("S-001");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "Test Sample");
    EXPECT_EQ(found->stock, 100);
}

TEST_F(SampleRepositoryTest, UpdateSampleStock)
{
    SampleRepository repo(SAMPLE_TEST_FILE);
    Sample s("S-001", "Test Sample", 0.5, 0.9, 100);
    repo.save(s);
    s.stock = 200;
    repo.update(s);
    auto found = repo.findById("S-001");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->stock, 200);
}

TEST_F(SampleRepositoryTest, FindAllReturnsSavedSamples)
{
    SampleRepository repo(SAMPLE_TEST_FILE);
    repo.save(Sample("S-001", "Sample A", 0.5, 0.9, 100));
    repo.save(Sample("S-002", "Sample B", 0.3, 0.85, 50));
    auto all = repo.findAll();
    EXPECT_EQ(all.size(), 2u);
}

TEST_F(SampleRepositoryTest, RemoveSampleById)
{
    SampleRepository repo(SAMPLE_TEST_FILE);
    repo.save(Sample("S-001", "Test Sample", 0.5, 0.9, 100));
    repo.remove("S-001");
    auto found = repo.findById("S-001");
    EXPECT_FALSE(found.has_value());
}

TEST_F(SampleRepositoryTest, FindByIdReturnsNulloptWhenNotFound)
{
    SampleRepository repo(SAMPLE_TEST_FILE);
    auto found = repo.findById("NONEXISTENT");
    EXPECT_FALSE(found.has_value());
}

// ========== OrderRepository Tests ==========

static const char* ORDER_TEST_FILE = "test_orders_tmp.json";

class OrderRepositoryTest : public ::testing::Test {
protected:
    void TearDown() override {
        std::remove(ORDER_TEST_FILE);
    }
};

TEST_F(OrderRepositoryTest, SaveAndFindOrderById)
{
    OrderRepository repo(ORDER_TEST_FILE);
    Order o("ORD-001", "S-001", "CustomerA", 100);
    repo.save(o);
    auto found = repo.findById("ORD-001");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->customerName, "CustomerA");
}

TEST_F(OrderRepositoryTest, FindOrdersByStatus)
{
    OrderRepository repo(ORDER_TEST_FILE);
    repo.save(Order("ORD-001", "S-001", "CustomerA", 100));
    Order o2("ORD-002", "S-001", "CustomerB", 50);
    o2.changeStatus(OrderStatus::CONFIRMED);
    repo.save(o2);
    auto reserved = repo.findByStatus(OrderStatus::RESERVED);
    EXPECT_EQ(reserved.size(), 1u);
    EXPECT_EQ(reserved[0].orderId, "ORD-001");
}

TEST_F(OrderRepositoryTest, UpdateOrderStatus)
{
    OrderRepository repo(ORDER_TEST_FILE);
    Order o("ORD-001", "S-001", "Customer", 100);
    repo.save(o);
    o.changeStatus(OrderStatus::CONFIRMED);
    repo.update(o);
    auto found = repo.findById("ORD-001");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->status, OrderStatus::CONFIRMED);
}
