#pragma once
#include "Repository/SampleRepository.h"
#include "Repository/OrderRepository.h"
#include "Model/Order.h"
#include <iostream>
#include <iomanip>
#include <string>

class DataMonitor {
public:
    DataMonitor(const std::string& sampleFile, const std::string& orderFile)
        : sampleRepo_(sampleFile), orderRepo_(orderFile) {}

    void printSamples() {
        auto samples = sampleRepo_.findAll();
        std::cout << "=== Samples (" << samples.size() << ") ===\n";
        std::cout << std::left
                  << std::setw(8)  << "ID"
                  << std::setw(24) << "Name"
                  << std::setw(10) << "AvgTime"
                  << std::setw(8)  << "Yield"
                  << "Stock\n";
        for (auto& s : samples)
            std::cout << std::setw(8)  << s.id
                      << std::setw(24) << s.name
                      << std::setw(10) << s.avgProductionTime
                      << std::setw(8)  << s.yieldRate
                      << s.stock << "\n";
    }

    void printOrders(OrderStatus filter, bool useFilter = false) {
        auto orders = orderRepo_.findAll();
        std::cout << "=== Orders (" << orders.size() << ") ===\n";
        std::cout << std::left
                  << std::setw(22) << "OrderId"
                  << std::setw(10) << "SampleId"
                  << std::setw(16) << "Customer"
                  << std::setw(8)  << "Qty"
                  << "Status\n";
        for (auto& o : orders) {
            if (useFilter && o.status != filter) continue;
            std::cout << std::setw(22) << o.orderId
                      << std::setw(10) << o.sampleId
                      << std::setw(16) << o.customerName
                      << std::setw(8)  << o.quantity
                      << statusStr(o.status) << "\n";
        }
    }

    void printAll() {
        printSamples();
        std::cout << "\n";
        printOrders(OrderStatus::RESERVED, false);
    }

private:
    SampleRepository sampleRepo_;
    OrderRepository  orderRepo_;

    static std::string statusStr(OrderStatus s) {
        switch (s) {
        case OrderStatus::RESERVED:  return "RESERVED";
        case OrderStatus::CONFIRMED: return "CONFIRMED";
        case OrderStatus::PRODUCING: return "PRODUCING";
        case OrderStatus::REJECTED:  return "REJECTED";
        case OrderStatus::RELEASE:   return "RELEASE";
        default: return "UNKNOWN";
        }
    }
};
