#pragma once
#include <string>
#include <stdexcept>

struct Sample
{
    std::string id;
    std::string name;
    double      avgProductionTime;
    double      yieldRate;
    int         stock;

    Sample(const std::string& id,
           const std::string& name,
           double avgProductionTime,
           double yieldRate,
           int stock)
        : id(id), name(name),
          avgProductionTime(avgProductionTime),
          yieldRate(yieldRate),
          stock(stock)
    {
        if (avgProductionTime <= 0.0)
            throw std::invalid_argument("avgProductionTime must be positive");
        if (yieldRate < 0.0 || yieldRate > 1.0)
            throw std::invalid_argument("yieldRate must be between 0 and 1");
        if (stock < 0)
            throw std::invalid_argument("stock cannot be negative");
    }
};
