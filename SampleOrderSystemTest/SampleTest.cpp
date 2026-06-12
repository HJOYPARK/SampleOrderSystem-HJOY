#include <gtest/gtest.h>
#include "Model/Sample.h"

// Step 1-1: Sample 생성 기본 동작
TEST(SampleTest, CanCreateSampleWithValidFields)
{
    Sample s("S-001", "실리콘 웨이퍼-8인치", 0.5, 0.92, 480);
    EXPECT_EQ(s.id, "S-001");
    EXPECT_EQ(s.name, "실리콘 웨이퍼-8인치");
    EXPECT_DOUBLE_EQ(s.avgProductionTime, 0.5);
    EXPECT_DOUBLE_EQ(s.yieldRate, 0.92);
    EXPECT_EQ(s.stock, 480);
}

// Step 1-2: 수율 유효성 검사
TEST(SampleTest, YieldRateMustBeBetweenZeroAndOne)
{
    EXPECT_THROW(Sample("S-002", "test", 0.5, 1.5, 0), std::invalid_argument);
    EXPECT_THROW(Sample("S-002", "test", 0.5, -0.1, 0), std::invalid_argument);
}

// Step 1-3: 평균 생산시간 유효성 검사
TEST(SampleTest, AvgProductionTimeMustBePositive)
{
    EXPECT_THROW(Sample("S-003", "test", 0.0, 0.9, 0), std::invalid_argument);
    EXPECT_THROW(Sample("S-003", "test", -1.0, 0.9, 0), std::invalid_argument);
}

// Step 1-4: 재고 음수 방지
TEST(SampleTest, StockCannotBeNegative)
{
    EXPECT_THROW(Sample("S-004", "test", 0.5, 0.9, -1), std::invalid_argument);
}
