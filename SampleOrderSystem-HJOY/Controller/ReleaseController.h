#pragma once
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/Order.h"
#include <vector>

class ReleaseController {
public:
    ReleaseController(IRepository<Order>* orderRepo,
                      IRepository<Sample>* sampleRepo)
        : orderRepo_(orderRepo), sampleRepo_(sampleRepo) {}

    std::vector<Order> getReleasableOrders() {
        std::vector<Order> result;
        for (auto& o : orderRepo_->findAll())
            if (o.status == OrderStatus::CONFIRMED) result.push_back(o);
        return result;
    }

    bool releaseOrder(const std::string& orderId) {
        auto orderOpt = orderRepo_->findById(orderId);
        if (!orderOpt.has_value()) return false;
        if (orderOpt->status != OrderStatus::CONFIRMED) return false;

        Order order = *orderOpt;
        order.changeStatus(OrderStatus::RELEASE);
        orderRepo_->update(order);
        return true;
    }

private:
    IRepository<Order>*  orderRepo_;
    IRepository<Sample>* sampleRepo_;
};
