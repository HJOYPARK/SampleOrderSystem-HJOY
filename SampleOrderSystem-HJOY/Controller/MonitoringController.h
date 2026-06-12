#pragma once
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/Order.h"
#include <map>
#include <string>
#include <vector>

enum class StockStatus { SUFFICIENT, SHORTAGE, DEPLETED };

class MonitoringController {
public:
    MonitoringController(IRepository<Order>* orderRepo,
                         IRepository<Sample>* sampleRepo)
        : orderRepo_(orderRepo), sampleRepo_(sampleRepo) {}

    // Count orders by status, excluding REJECTED
    std::map<OrderStatus, int> getOrderCounts() {
        std::map<OrderStatus, int> counts;
        for (auto& o : orderRepo_->findAll()) {
            if (o.status != OrderStatus::REJECTED)
                counts[o.status]++;
        }
        return counts;
    }

    // Stock status per sample ID: DEPLETED(0), SHORTAGE(<pending), SUFFICIENT(>=pending)
    std::map<std::string, StockStatus> getStockStatuses() {
        auto samples = sampleRepo_->findAll();
        auto orders  = orderRepo_->findAll();

        // Calculate pending quantity per sample (CONFIRMED + PRODUCING)
        std::map<std::string, int> pendingQty;
        for (auto& o : orders) {
            if (o.status == OrderStatus::CONFIRMED || o.status == OrderStatus::PRODUCING)
                pendingQty[o.sampleId] += o.quantity;
        }

        std::map<std::string, StockStatus> result;
        for (auto& s : samples) {
            if (s.stock == 0) {
                result[s.id] = StockStatus::DEPLETED;
            } else {
                int pending = pendingQty.count(s.id) ? pendingQty[s.id] : 0;
                result[s.id] = (s.stock >= pending) ? StockStatus::SUFFICIENT : StockStatus::SHORTAGE;
            }
        }
        return result;
    }

private:
    IRepository<Order>*  orderRepo_;
    IRepository<Sample>* sampleRepo_;
};
