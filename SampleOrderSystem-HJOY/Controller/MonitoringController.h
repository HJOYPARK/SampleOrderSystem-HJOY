#pragma once
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/Order.h"
#include <map>
#include <string>
#include <vector>

enum class StockStatus { SUFFICIENT, SHORTAGE, DEPLETED };

struct SampleStockInfo {
    std::string id;
    std::string name;
    int         stock;
    int         pendingQty; // CONFIRMED + PRODUCING 주문 수량 합계
    StockStatus status;

    // 잔여율: 재고가 대기 수량을 얼마나 커버하는지 (0.0 ~ 1.0)
    double coverageRatio() const {
        if (pendingQty <= 0) return stock > 0 ? 1.0 : 0.0;
        double r = static_cast<double>(stock) / pendingQty;
        return r < 1.0 ? r : 1.0;
    }
};

class MonitoringController {
public:
    MonitoringController(IRepository<Order>* orderRepo,
                         IRepository<Sample>* sampleRepo)
        : orderRepo_(orderRepo), sampleRepo_(sampleRepo) {}

    // 상태별 주문 건수 (REJECTED 제외)
    std::map<OrderStatus, int> getOrderCounts() {
        std::map<OrderStatus, int> counts;
        for (auto& o : orderRepo_->findAll()) {
            if (o.status != OrderStatus::REJECTED)
                counts[o.status]++;
        }
        return counts;
    }

    // 시료별 재고 현황 (이름·재고량·대기수량·상태 포함)
    std::vector<SampleStockInfo> getStockInfos() {
        auto samples = sampleRepo_->findAll();
        auto orders  = orderRepo_->findAll();

        std::map<std::string, int> pendingQty;
        for (auto& o : orders) {
            if (o.status == OrderStatus::CONFIRMED || o.status == OrderStatus::PRODUCING)
                pendingQty[o.sampleId] += o.quantity;
        }

        std::vector<SampleStockInfo> result;
        for (auto& s : samples) {
            SampleStockInfo info;
            info.id         = s.id;
            info.name       = s.name;
            info.stock      = s.stock;
            info.pendingQty = pendingQty.count(s.id) ? pendingQty.at(s.id) : 0;

            if (s.stock == 0)
                info.status = StockStatus::DEPLETED;
            else if (s.stock >= info.pendingQty)
                info.status = StockStatus::SUFFICIENT;
            else
                info.status = StockStatus::SHORTAGE;

            result.push_back(info);
        }
        return result;
    }

    // 하위 호환용 (테스트에서 사용)
    std::map<std::string, StockStatus> getStockStatuses() {
        std::map<std::string, StockStatus> result;
        for (auto& info : getStockInfos())
            result[info.id] = info.status;
        return result;
    }

private:
    IRepository<Order>*  orderRepo_;
    IRepository<Sample>* sampleRepo_;
};
