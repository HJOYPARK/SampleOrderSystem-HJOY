#pragma once
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/Order.h"
#include "Model/ProductionJob.h"
#include "Model/IProductionQueue.h"
#include <optional>
#include <vector>
#include <stdexcept>

class ProductionController {
public:
    ProductionController(IProductionQueue* queue,
                         IRepository<Sample>* sampleRepo,
                         IRepository<Order>* orderRepo)
        : queue_(queue), sampleRepo_(sampleRepo), orderRepo_(orderRepo) {}

    std::optional<ProductionJob> getCurrentJob() {
        return queue_->front();
    }

    // Returns all jobs except the first (waiting queue)
    std::vector<ProductionJob> getWaitingJobs() {
        auto all = queue_->getAllJobs();
        if (all.size() <= 1) return {};
        return std::vector<ProductionJob>(all.begin() + 1, all.end());
    }

    std::vector<ProductionJob> getAllJobs() {
        return queue_->getAllJobs();
    }

    bool completeCurrentProduction() {
        auto jobOpt = queue_->front();
        if (!jobOpt.has_value()) return false;

        const ProductionJob& job = *jobOpt;
        auto orderOpt = orderRepo_->findById(job.orderId);
        auto sampleOpt = sampleRepo_->findById(job.sampleId);

        if (!orderOpt.has_value() || !sampleOpt.has_value()) return false;

        // Increase stock by actual production
        Sample sample = *sampleOpt;
        sample.stock += job.actualProduction;
        sampleRepo_->update(sample);

        // Transition order to CONFIRMED
        Order order = *orderOpt;
        order.changeStatus(OrderStatus::CONFIRMED);
        orderRepo_->update(order);

        queue_->dequeue();
        return true;
    }

private:
    IProductionQueue*    queue_;
    IRepository<Sample>* sampleRepo_;
    IRepository<Order>*  orderRepo_;
};
