#include <gtest/gtest.h>
#include "Model/ProductionJob.h"

// Step 3-1: Actual production calculation
// actualProduction = ceil(shortage / (yieldRate * 0.9))
TEST(ProductionJobTest, CalculatesActualProductionWithYieldAndBuffer)
{
    // shortage=170, yieldRate=0.92 => ceil(170 / 0.828) = ceil(205.31) = 206
    ProductionJob job("ORD-001", "S-003", 170, 0.92, 0.8);
    EXPECT_EQ(job.actualProduction, 206);
}

// Step 3-2: Total production time calculation
// totalProductionTime = avgProductionTime * actualProduction
TEST(ProductionJobTest, CalculatesTotalProductionTime)
{
    // 0.8 * 206 = 164.8
    ProductionJob job("ORD-001", "S-003", 170, 0.92, 0.8);
    EXPECT_DOUBLE_EQ(job.totalProductionTime, 164.8);
}

// Step 3-3: Zero shortage is invalid
TEST(ProductionJobTest, ZeroShortageProducesZeroActualProduction)
{
    EXPECT_THROW(ProductionJob("ORD-001", "S-001", 0, 0.92, 0.5), std::invalid_argument);
}

// Step 3-4: Shortage calculation helper
TEST(ProductionJobTest, ShortageCalculatedAsOrderQuantityMinusCurrentStock)
{
    // order=200, stock=30 => shortage=170
    int shortage = ProductionJob::calcShortage(200, 30);
    EXPECT_EQ(shortage, 170);
}

TEST(ProductionJobTest, NoShortageWhenStockCoversOrder)
{
    int shortage = ProductionJob::calcShortage(100, 150);
    EXPECT_EQ(shortage, 0);
}
