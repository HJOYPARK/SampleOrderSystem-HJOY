#pragma once
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/Order.h"
#include "Model/ProductionJob.h"
#include "Model/IProductionQueue.h"
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>

class OrderController {
public:
    OrderController(IRepository<Sample>* sampleRepo,
                    IRepository<Order>* orderRepo,
                    IProductionQueue* queue = nullptr)
        : sampleRepo_(sampleRepo), orderRepo_(orderRepo), queue_(queue) {}

    std::string placeOrder(const std::string& sampleId,
                           const std::string& customerName, int quantity) {
        if (!sampleRepo_->findById(sampleId).has_value()) return "";

        std::string date = currentDateString();
        int seq = static_cast<int>(orderRepo_->findAll().size()) + 1;
        std::string orderId = Order::generateOrderId(date, seq);

        orderRepo_->save(Order(orderId, sampleId, customerName, quantity));
        return orderId;
    }

    OrderStatus approveOrder(const std::string& orderId) {
        auto orderOpt = orderRepo_->findById(orderId);
        if (!orderOpt.has_value()) throw std::runtime_error("Order not found");

        auto sampleOpt = sampleRepo_->findById(orderOpt->sampleId);
        if (!sampleOpt.has_value()) throw std::runtime_error("Sample not found");

        Order order = *orderOpt;
        Sample sample = *sampleOpt;
        int shortage = ProductionJob::calcShortage(order.quantity, sample.stock);

        if (shortage == 0) {
            // Stock sufficient -> CONFIRMED, deduct stock
            sample.stock -= order.quantity;
            sampleRepo_->update(sample);
            order.changeStatus(OrderStatus::CONFIRMED);
            orderRepo_->update(order);
            return OrderStatus::CONFIRMED;
        } else {
            // Stock insufficient -> PRODUCING, enqueue production job
            order.changeStatus(OrderStatus::PRODUCING);
            orderRepo_->update(order);
            if (queue_) {
                ProductionJob job(orderId, order.sampleId, shortage,
                                  sample.yieldRate, sample.avgProductionTime);
                queue_->enqueue(job);
            }
            return OrderStatus::PRODUCING;
        }
    }

    bool rejectOrder(const std::string& orderId) {
        auto orderOpt = orderRepo_->findById(orderId);
        if (!orderOpt.has_value()) return false;
        Order order = *orderOpt;
        order.changeStatus(OrderStatus::REJECTED);
        orderRepo_->update(order);
        return true;
    }

    std::vector<Order> getPendingOrders() {
        std::vector<Order> result;
        for (auto& o : orderRepo_->findAll())
            if (o.status == OrderStatus::RESERVED) result.push_back(o);
        return result;
    }

    std::vector<Order> getAllOrders() {
        return orderRepo_->findAll();
    }

private:
    IRepository<Sample>* sampleRepo_;
    IRepository<Order>*  orderRepo_;
    IProductionQueue*    queue_;

    std::string currentDateString() {
        std::time_t t = std::time(nullptr);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y%m%d");
        return oss.str();
    }
};
