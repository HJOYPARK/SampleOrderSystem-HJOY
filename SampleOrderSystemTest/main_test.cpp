#include <gtest/gtest.h>
#include <gmock/gmock.h>

int main(int argc, char** argv)
{
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(EnvironmentTest, GoogleTestWorks)
{
    EXPECT_EQ(1, 1);
}

TEST(EnvironmentTest, GoogleMockWorks)
{
    EXPECT_TRUE(true);
}
