#pragma once
#include <string>
#include <stdexcept>
#include <cmath>

struct ProductionJob {
    std::string orderId;
    std::string sampleId;
    int    shortage;
    double yieldRate;
    double avgProductionTime;
    int    actualProduction;
    double totalProductionTime;

    ProductionJob(const std::string& orderId, const std::string& sampleId,
                  int shortage, double yieldRate, double avgProductionTime)
        : orderId(orderId), sampleId(sampleId),
          shortage(shortage), yieldRate(yieldRate),
          avgProductionTime(avgProductionTime)
    {
        if (shortage <= 0)
            throw std::invalid_argument("shortage must be positive");

        // actualProduction = ceil(shortage / (yieldRate * 0.9))
        actualProduction = static_cast<int>(
            std::ceil(static_cast<double>(shortage) / (yieldRate * 0.9)));

        totalProductionTime = avgProductionTime * actualProduction;
    }

    static int calcShortage(int orderQuantity, int currentStock) {
        int diff = orderQuantity - currentStock;
        return diff > 0 ? diff : 0;
    }
};
