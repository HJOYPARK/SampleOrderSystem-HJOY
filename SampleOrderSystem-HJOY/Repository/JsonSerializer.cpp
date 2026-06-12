#include "JsonSerializer.h"
#include <sstream>
#include <stdexcept>

// ---- helpers ----

std::string JsonSerializer::escapeString(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

// Extract value for "key":"value" patterns
std::string JsonSerializer::extractStringValue(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\":\"";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return "";
    pos += needle.size();
    std::string result;
    while (pos < json.size() && !(json[pos] == '"' && (pos == 0 || json[pos-1] != '\\'))) {
        result += json[pos++];
    }
    return result;
}

double JsonSerializer::extractDoubleValue(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\":";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return 0.0;
    pos += needle.size();
    std::string num;
    while (pos < json.size() && (std::isdigit(json[pos]) || json[pos] == '.' || json[pos] == '-' || json[pos] == 'e' || json[pos] == 'E' || json[pos] == '+'))
        num += json[pos++];
    return std::stod(num);
}

int JsonSerializer::extractIntValue(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\":";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return 0;
    pos += needle.size();
    std::string num;
    while (pos < json.size() && (std::isdigit(json[pos]) || json[pos] == '-'))
        num += json[pos++];
    if (num.empty()) return 0;
    return std::stoi(num);
}

long long JsonSerializer::extractLongLongValue(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\":";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return 0LL;
    pos += needle.size();
    std::string num;
    while (pos < json.size() && (std::isdigit(json[pos]) || json[pos] == '-'))
        num += json[pos++];
    if (num.empty()) return 0LL;
    return std::stoll(num);
}

std::string JsonSerializer::statusToString(OrderStatus s) {
    switch (s) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::REJECTED:  return "REJECTED";
    case OrderStatus::RELEASE:   return "RELEASE";
    default: return "RESERVED";
    }
}

OrderStatus JsonSerializer::stringToStatus(const std::string& s) {
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "RELEASE")   return OrderStatus::RELEASE;
    return OrderStatus::RESERVED;
}

// ---- Sample ----

std::string JsonSerializer::toJson(const Sample& s) {
    std::ostringstream oss;
    oss << "{\"id\":\"" << escapeString(s.id) << "\""
        << ",\"name\":\"" << escapeString(s.name) << "\""
        << ",\"avgProductionTime\":" << s.avgProductionTime
        << ",\"yieldRate\":" << s.yieldRate
        << ",\"stock\":" << s.stock
        << "}";
    return oss.str();
}

Sample JsonSerializer::sampleFromJson(const std::string& json) {
    std::string id   = extractStringValue(json, "id");
    std::string name = extractStringValue(json, "name");
    double apt       = extractDoubleValue(json, "avgProductionTime");
    double yr        = extractDoubleValue(json, "yieldRate");
    int stock        = extractIntValue(json, "stock");
    return Sample(id, name, apt, yr, stock);
}

// ---- Order ----

std::string JsonSerializer::toJson(const Order& o) {
    std::ostringstream oss;
    oss << "{\"orderId\":\"" << escapeString(o.orderId) << "\""
        << ",\"sampleId\":\"" << escapeString(o.sampleId) << "\""
        << ",\"customerName\":\"" << escapeString(o.customerName) << "\""
        << ",\"quantity\":" << o.quantity
        << ",\"status\":\"" << statusToString(o.status) << "\""
        << ",\"productionStartTime\":" << static_cast<long long>(o.productionStartTime)
        << ",\"shortage\":" << o.shortage
        << ",\"actualProduction\":" << o.actualProduction
        << ",\"producedCount\":" << o.producedCount
        << "}";
    return oss.str();
}

Order JsonSerializer::orderFromJson(const std::string& json) {
    std::string orderId      = extractStringValue(json, "orderId");
    std::string sampleId     = extractStringValue(json, "sampleId");
    std::string customerName = extractStringValue(json, "customerName");
    int quantity             = extractIntValue(json, "quantity");
    std::string statusStr    = extractStringValue(json, "status");

    Order o(orderId, sampleId, customerName, quantity);
    OrderStatus target = stringToStatus(statusStr);
    if (target != OrderStatus::RESERVED) {
        if (target == OrderStatus::CONFIRMED || target == OrderStatus::PRODUCING || target == OrderStatus::REJECTED) {
            o.changeStatus(target);
        } else if (target == OrderStatus::RELEASE) {
            o.changeStatus(OrderStatus::CONFIRMED);
            o.changeStatus(OrderStatus::RELEASE);
        }
    }
    o.productionStartTime = static_cast<std::time_t>(extractLongLongValue(json, "productionStartTime"));
    o.shortage            = extractIntValue(json, "shortage");
    o.actualProduction    = extractIntValue(json, "actualProduction");
    o.producedCount       = extractIntValue(json, "producedCount");
    return o;
}
