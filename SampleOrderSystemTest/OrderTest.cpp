#include <gtest/gtest.h>
#include "Model/Order.h"

// Step 2-1: Order creation with RESERVED status
TEST(OrderTest, CanCreateOrderWithReservedStatus)
{
    Order o("ORD-20260416-0001", "S-001", "Samsung", 200);
    EXPECT_EQ(o.orderId, "ORD-20260416-0001");
    EXPECT_EQ(o.sampleId, "S-001");
    EXPECT_EQ(o.customerName, "Samsung");
    EXPECT_EQ(o.quantity, 200);
    EXPECT_EQ(o.status, OrderStatus::RESERVED);
}

// Step 2-2: Quantity validation
TEST(OrderTest, QuantityMustBePositive)
{
    EXPECT_THROW(Order("ORD-20260416-0002", "S-001", "Customer", 0), std::invalid_argument);
    EXPECT_THROW(Order("ORD-20260416-0002", "S-001", "Customer", -5), std::invalid_argument);
}

// Step 2-3: Order ID format generator
TEST(OrderTest, GenerateOrderIdWithCorrectFormat)
{
    std::string id = Order::generateOrderId("20260416", 43);
    EXPECT_EQ(id, "ORD-20260416-0043");
}

TEST(OrderTest, GenerateOrderIdPadsSequenceToFourDigits)
{
    std::string id = Order::generateOrderId("20260416", 1);
    EXPECT_EQ(id, "ORD-20260416-0001");
}

// Step 2-4: Status transition validation
TEST(OrderTest, CanTransitionFromReservedToConfirmed)
{
    Order o("ORD-20260416-0001", "S-001", "Customer", 100);
    EXPECT_NO_THROW(o.changeStatus(OrderStatus::CONFIRMED));
    EXPECT_EQ(o.status, OrderStatus::CONFIRMED);
}

TEST(OrderTest, CanTransitionFromReservedToProducing)
{
    Order o("ORD-20260416-0001", "S-001", "Customer", 100);
    EXPECT_NO_THROW(o.changeStatus(OrderStatus::PRODUCING));
    EXPECT_EQ(o.status, OrderStatus::PRODUCING);
}

TEST(OrderTest, CanTransitionFromReservedToRejected)
{
    Order o("ORD-20260416-0001", "S-001", "Customer", 100);
    EXPECT_NO_THROW(o.changeStatus(OrderStatus::REJECTED));
    EXPECT_EQ(o.status, OrderStatus::REJECTED);
}

TEST(OrderTest, CanTransitionFromConfirmedToRelease)
{
    Order o("ORD-20260416-0001", "S-001", "Customer", 100);
    o.changeStatus(OrderStatus::CONFIRMED);
    EXPECT_NO_THROW(o.changeStatus(OrderStatus::RELEASE));
    EXPECT_EQ(o.status, OrderStatus::RELEASE);
}

TEST(OrderTest, CanTransitionFromProducingToRelease)
{
    Order o("ORD-20260416-0001", "S-001", "Customer", 100);
    o.changeStatus(OrderStatus::PRODUCING);
    EXPECT_NO_THROW(o.changeStatus(OrderStatus::RELEASE));
}

TEST(OrderTest, CannotTransitionFromReleaseToAnyOtherStatus)
{
    Order o("ORD-20260416-0001", "S-001", "Customer", 100);
    o.changeStatus(OrderStatus::CONFIRMED);
    o.changeStatus(OrderStatus::RELEASE);
    EXPECT_THROW(o.changeStatus(OrderStatus::RESERVED), std::logic_error);
}

TEST(OrderTest, CannotTransitionFromRejectedToAnyStatus)
{
    Order o("ORD-20260416-0001", "S-001", "Customer", 100);
    o.changeStatus(OrderStatus::REJECTED);
    EXPECT_THROW(o.changeStatus(OrderStatus::CONFIRMED), std::logic_error);
}
