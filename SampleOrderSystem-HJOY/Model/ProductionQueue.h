#pragma once
#include "IProductionQueue.h"
#include <queue>
#include <vector>
#include <optional>

class ProductionQueue : public IProductionQueue {
public:
    void enqueue(const ProductionJob& job) override {
        queue_.push(job);
    }

    std::optional<ProductionJob> front() const override {
        if (queue_.empty()) return std::nullopt;
        return queue_.front();
    }

    void dequeue() override {
        if (!queue_.empty()) queue_.pop();
    }

    std::vector<ProductionJob> getAllJobs() const override {
        std::vector<ProductionJob> result;
        auto copy = queue_;
        while (!copy.empty()) {
            result.push_back(copy.front());
            copy.pop();
        }
        return result;
    }

    bool empty() const override {
        return queue_.empty();
    }

private:
    std::queue<ProductionJob> queue_;
};
