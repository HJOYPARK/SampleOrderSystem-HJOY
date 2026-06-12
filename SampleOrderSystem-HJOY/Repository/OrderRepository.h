#pragma once
#include "IRepository.h"
#include "JsonSerializer.h"
#include "Model/Order.h"
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <algorithm>

class OrderRepository : public IRepository<Order> {
public:
    explicit OrderRepository(const std::string& filePath) : filePath_(filePath) {}

    void save(const Order& entity) override {
        auto all = loadAll();
        all.push_back(entity);
        writeAll(all);
    }

    std::optional<Order> findById(const std::string& id) override {
        for (auto& o : loadAll())
            if (o.orderId == id) return o;
        return std::nullopt;
    }

    std::vector<Order> findAll() override {
        return loadAll();
    }

    void update(const Order& entity) override {
        auto all = loadAll();
        for (auto& o : all)
            if (o.orderId == entity.orderId) { o = entity; break; }
        writeAll(all);
    }

    void remove(const std::string& id) override {
        auto all = loadAll();
        all.erase(std::remove_if(all.begin(), all.end(),
            [&](const Order& o) { return o.orderId == id; }), all.end());
        writeAll(all);
    }

    std::vector<Order> findByStatus(OrderStatus status) {
        std::vector<Order> result;
        for (auto& o : loadAll())
            if (o.status == status) result.push_back(o);
        return result;
    }

private:
    std::string filePath_;

    std::vector<Order> loadAll() {
        std::ifstream f(filePath_);
        if (!f.is_open()) return {};
        std::string content((std::istreambuf_iterator<char>(f)), {});
        return parseJsonArray(content);
    }

    void writeAll(const std::vector<Order>& items) {
        std::ofstream f(filePath_);
        f << "[";
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) f << ",";
            f << JsonSerializer::toJson(items[i]);
        }
        f << "]";
    }

    std::vector<Order> parseJsonArray(const std::string& content) {
        std::vector<Order> result;
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
                result.push_back(JsonSerializer::orderFromJson(content.substr(start, i - start)));
            } catch (...) {}
            pos = i;
        }
        return result;
    }
};
