#pragma once
#include "IRepository.h"
#include "JsonSerializer.h"
#include "Model/Sample.h"
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <algorithm>

class SampleRepository : public IRepository<Sample> {
public:
    explicit SampleRepository(const std::string& filePath) : filePath_(filePath) {}

    void save(const Sample& entity) override {
        auto all = loadAll();
        all.push_back(entity);
        writeAll(all);
    }

    std::optional<Sample> findById(const std::string& id) override {
        for (auto& s : loadAll())
            if (s.id == id) return s;
        return std::nullopt;
    }

    std::vector<Sample> findAll() override {
        return loadAll();
    }

    void update(const Sample& entity) override {
        auto all = loadAll();
        for (auto& s : all)
            if (s.id == entity.id) { s = entity; break; }
        writeAll(all);
    }

    void remove(const std::string& id) override {
        auto all = loadAll();
        all.erase(std::remove_if(all.begin(), all.end(),
            [&](const Sample& s) { return s.id == id; }), all.end());
        writeAll(all);
    }

private:
    std::string filePath_;

    std::vector<Sample> loadAll() {
        std::ifstream f(filePath_);
        if (!f.is_open()) return {};
        std::string content((std::istreambuf_iterator<char>(f)), {});
        return parseJsonArray(content);
    }

    void writeAll(const std::vector<Sample>& items) {
        std::ofstream f(filePath_);
        f << "[";
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) f << ",";
            f << JsonSerializer::toJson(items[i]);
        }
        f << "]";
    }

    std::vector<Sample> parseJsonArray(const std::string& content) {
        std::vector<Sample> result;
        size_t pos = 0;
        while (pos < content.size()) {
            auto start = content.find('{', pos);
            if (start == std::string::npos) break;
            int depth = 1;
            size_t i = start + 1;
            while (i < content.size() && depth > 0) {
                if (content[i] == '{') ++depth;
                else if (content[i] == '}') --depth;
                ++i;
            }
            try {
                result.push_back(JsonSerializer::sampleFromJson(content.substr(start, i - start)));
            } catch (...) {}
            pos = i;
        }
        return result;
    }
};
