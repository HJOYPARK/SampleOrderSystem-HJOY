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
#include <algorithm>

class ProductionController {
public:
    ProductionController(IProductionQueue* queue,
                         IRepository<Sample>* sampleRepo,
                         IRepository<Order>* orderRepo)
        : queue_(queue), sampleRepo_(sampleRepo), orderRepo_(orderRepo) {}

    std::optional<ProductionJob> getCurrentJob() {
        return queue_->front();
    }

    std::vector<ProductionJob> getWaitingJobs() {
        auto all = queue_->getAllJobs();
        if (all.size() <= 1) return {};
        return std::vector<ProductionJob>(all.begin() + 1, all.end());
    }

    std::vector<ProductionJob> getAllJobs() {
        return queue_->getAllJobs();
    }

    // 현재까지 완료된 생산 수량 (실시간 계산)
    int getLiveProducedCount() const {
        auto jobOpt = queue_->front();
        if (!jobOpt.has_value()) return 0;
        auto orderOpt = orderRepo_->findById(jobOpt->orderId);
        if (!orderOpt.has_value() || orderOpt->productionStartTime == 0) return 0;

        std::time_t now = std::time(nullptr);
        double elapsedMin = std::difftime(now, orderOpt->productionStartTime) / 60.0;
        int completed = static_cast<int>(elapsedMin / jobOpt->avgProductionTime);
        return std::min(completed, jobOpt->actualProduction);
    }

    // 진행률 (0.0 ~ 1.0)
    double getCurrentJobProgress() const {
        auto jobOpt = queue_->front();
        if (!jobOpt.has_value() || jobOpt->actualProduction == 0) return 0.0;
        double ratio = static_cast<double>(getLiveProducedCount()) / jobOpt->actualProduction;
        return ratio < 1.0 ? ratio : 1.0;
    }

    // 경과 시간마다 완료된 개별 단위를 즉시 재고에 반영하고, 전체 완료 시 CONFIRMED로 전환
    void syncProduction() {
        while (!queue_->empty()) {
            auto jobOpt = queue_->front();
            if (!jobOpt.has_value()) break;

            const ProductionJob job = *jobOpt;
            auto orderOpt = orderRepo_->findById(job.orderId);
            if (!orderOpt.has_value()) { queue_->dequeue(); continue; }

            Order order = *orderOpt;
            if (order.productionStartTime == 0) break;

            // 경과 시간으로 완료된 수량 계산 (개당 avgProductionTime 분)
            std::time_t now = std::time(nullptr);
            double elapsedMin = std::difftime(now, order.productionStartTime) / 60.0;
            int completedUnits = static_cast<int>(elapsedMin / job.avgProductionTime);
            if (completedUnits > job.actualProduction) completedUnits = job.actualProduction;

            // 새로 완료된 수량만 재고에 추가
            int delta = completedUnits - order.producedCount;
            if (delta > 0) {
                auto sampleOpt = sampleRepo_->findById(job.sampleId);
                if (sampleOpt.has_value()) {
                    Sample sample = *sampleOpt;
                    sample.stock += delta;
                    sampleRepo_->update(sample);
                }
                order.producedCount = completedUnits;
                orderRepo_->update(order);
            }

            // 전체 생산 완료 → CONFIRMED 전환, 큐에서 제거
            if (completedUnits >= job.actualProduction) {
                order.changeStatus(OrderStatus::CONFIRMED);
                orderRepo_->update(order);
                queue_->dequeue();
                // 다음 작업도 연속 확인
            } else {
                break;
            }
        }
    }

    // 수동 즉시 완료 (잔여량만 재고 추가, 이미 반영된 수량 중복 방지)
    bool completeCurrentProduction() {
        auto jobOpt = queue_->front();
        if (!jobOpt.has_value()) return false;

        const ProductionJob job = *jobOpt;
        auto orderOpt = orderRepo_->findById(job.orderId);
        auto sampleOpt = sampleRepo_->findById(job.sampleId);
        if (!orderOpt.has_value() || !sampleOpt.has_value()) return false;

        Order  order  = *orderOpt;
        Sample sample = *sampleOpt;

        // producedCount 이후 잔여량만 추가
        int remaining = job.actualProduction - order.producedCount;
        if (remaining > 0) {
            sample.stock += remaining;
            sampleRepo_->update(sample);
        }
        order.producedCount = job.actualProduction;
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
