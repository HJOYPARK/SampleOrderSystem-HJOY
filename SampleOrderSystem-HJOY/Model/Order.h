#pragma once
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>

enum class OrderStatus {
    RESERVED,
    CONFIRMED,
    PRODUCING,
    REJECTED,
    RELEASE
};

struct Order {
    std::string orderId;
    std::string sampleId;
    std::string customerName;
    int         quantity;
    OrderStatus status;

    Order(const std::string& orderId, const std::string& sampleId,
          const std::string& customerName, int quantity)
        : orderId(orderId), sampleId(sampleId),
          customerName(customerName), quantity(quantity),
          status(OrderStatus::RESERVED)
    {
        if (quantity <= 0)
            throw std::invalid_argument("quantity must be positive");
    }

    static std::string generateOrderId(const std::string& date, int sequence) {
        std::ostringstream oss;
        oss << "ORD-" << date << "-" << std::setw(4) << std::setfill('0') << sequence;
        return oss.str();
    }

    void changeStatus(OrderStatus newStatus) {
        if (status == OrderStatus::RELEASE)
            throw std::logic_error("Cannot change status from RELEASE");
        if (status == OrderStatus::REJECTED)
            throw std::logic_error("Cannot change status from REJECTED");

        bool valid = false;
        switch (status) {
        case OrderStatus::RESERVED:
            valid = (newStatus == OrderStatus::CONFIRMED ||
                     newStatus == OrderStatus::PRODUCING ||
                     newStatus == OrderStatus::REJECTED);
            break;
        case OrderStatus::CONFIRMED:
            valid = (newStatus == OrderStatus::RELEASE);
            break;
        case OrderStatus::PRODUCING:
            // After production completes: CONFIRMED (stock now sufficient) or RELEASE
            valid = (newStatus == OrderStatus::CONFIRMED ||
                     newStatus == OrderStatus::RELEASE);
            break;
        default:
            break;
        }

        if (!valid)
            throw std::logic_error("Invalid status transition");

        status = newStatus;
    }
};
