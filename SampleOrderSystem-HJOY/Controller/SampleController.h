#pragma once
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include <string>
#include <vector>

class SampleController {
public:
    explicit SampleController(IRepository<Sample>* repo) : repo_(repo) {}

    bool registerSample(const std::string& id, const std::string& name,
                        double avgProductionTime, double yieldRate, int stock) {
        if (repo_->findById(id).has_value()) return false;
        repo_->save(Sample(id, name, avgProductionTime, yieldRate, stock));
        return true;
    }

    std::vector<Sample> listSamples() {
        return repo_->findAll();
    }

    std::vector<Sample> searchByName(const std::string& keyword) {
        std::vector<Sample> result;
        for (auto& s : repo_->findAll())
            if (s.name.find(keyword) != std::string::npos)
                result.push_back(s);
        return result;
    }

    bool updateStock(const std::string& id, int newStock) {
        auto found = repo_->findById(id);
        if (!found.has_value()) return false;
        found->stock = newStock;
        repo_->update(*found);
        return true;
    }

    bool removeSample(const std::string& id) {
        if (!repo_->findById(id).has_value()) return false;
        repo_->remove(id);
        return true;
    }

private:
    IRepository<Sample>* repo_;
};
