#pragma once
#include "ProductionJob.h"
#include <optional>
#include <vector>

class IProductionQueue {
public:
    virtual void enqueue(const ProductionJob& job) = 0;
    virtual std::optional<ProductionJob> front() const = 0;
    virtual void dequeue() = 0;
    virtual std::vector<ProductionJob> getAllJobs() const = 0;
    virtual bool empty() const = 0;
    virtual ~IProductionQueue() = default;
};
