#pragma once
#include <string>
#include <optional>
#include <vector>

template<typename T>
class IRepository {
public:
    virtual void save(const T& entity) = 0;
    virtual std::optional<T> findById(const std::string& id) = 0;
    virtual std::vector<T> findAll() = 0;
    virtual void update(const T& entity) = 0;
    virtual void remove(const std::string& id) = 0;
    virtual ~IRepository() = default;
};
