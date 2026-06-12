#pragma once
#include <string>
#include "Model/Sample.h"
#include "Model/Order.h"

class JsonSerializer {
public:
    static std::string toJson(const Sample& s);
    static Sample sampleFromJson(const std::string& json);

    static std::string toJson(const Order& o);
    static Order orderFromJson(const std::string& json);

private:
    static std::string escapeString(const std::string& s);
    static std::string   extractStringValue  (const std::string& json, const std::string& key);
    static double        extractDoubleValue  (const std::string& json, const std::string& key);
    static int           extractIntValue     (const std::string& json, const std::string& key);
    static long long     extractLongLongValue(const std::string& json, const std::string& key);
    static std::string statusToString(OrderStatus s);
    static OrderStatus stringToStatus(const std::string& s);
};
