#pragma once
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/Order.h"
#include "Model/ProductionJob.h"
#include "Model/IProductionQueue.h"
#include <optional>
#include <vector>
#include <stdexcept>
#include <ctime>

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

    // 경과 시간 확인 후 완료된 작업을 자동으로 처리한다
    void syncProduction() {
        while (!queue_->empty()) {
            auto jobOpt = queue_->front();
            if (!jobOpt.has_value()) break;

            auto orderOpt = orderRepo_->findById(jobOpt->orderId);
            if (!orderOpt.has_value()) { queue_->dequeue(); continue; }

            if (orderOpt->productionStartTime == 0) break;

            std::time_t now = std::time(nullptr);
            double elapsedMin = std::difftime(now, orderOpt->productionStartTime) / 60.0;
            if (elapsedMin >= jobOpt->totalProductionTime)
                completeCurrentProduction();
            else
                break;
        }
    }

    // 현재 작업의 진행률 (0.0 ~ 1.0) 반환
    double getCurrentJobProgress() const {
        auto jobOpt = queue_->front();
        if (!jobOpt.has_value()) return 0.0;

        auto orderOpt = orderRepo_->findById(jobOpt->orderId);
        if (!orderOpt.has_value() || orderOpt->productionStartTime == 0) return 0.0;

        std::time_t now = std::time(nullptr);
        double elapsedMin = std::difftime(now, orderOpt->productionStartTime) / 60.0;
        double progress = elapsedMin / jobOpt->totalProductionTime;
        return progress < 1.0 ? progress : 1.0;
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
