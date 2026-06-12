#pragma once
#include "Repository/SampleRepository.h"
#include "Repository/OrderRepository.h"
#include "Model/Order.h"
#include <string>
#include <vector>

class DummyDataGenerator {
public:
    explicit DummyDataGenerator(const std::string& dataDir)
        : dataDir_(dataDir) {}

    void generateSamples(int count) {
        SampleRepository repo(dataDir_ + "samples.json");
        static const char* names[] = {
            "실리콘 웨이퍼-8인치",
            "GaN 에피택셀-4인치",
            "SiC 파워기판-6인치",
            "포토레지스트-PR7",
            "산화막 웨이퍼-SiO2",
            "InP 기판",
            "Ge 웨이퍼",
            "다이아몬드 기판",
            "AlGaAs 에피택셀",
            "SiGe 레이어"
        };
        static double times[]  = {0.5, 0.3, 0.8, 0.2, 0.6, 0.7, 0.4, 1.0, 0.35, 0.45};
        static double yields[] = {0.92, 0.78, 0.92, 0.95, 0.88, 0.82, 0.90, 0.75, 0.85, 0.80};
        static int stocks[]    = {480, 220, 30, 910, 0, 150, 300, 50, 180, 420};

        for (int i = 0; i < count && i < 10; ++i) {
            std::string id = "S-00" + std::to_string(i + 1);
            if (!repo.findById(id).has_value())
                repo.save(Sample(id, names[i], times[i], yields[i], stocks[i]));
        }
    }

    void generateOrders(int count) {
        OrderRepository repo(dataDir_ + "orders.json");
        SampleRepository sRepo(dataDir_ + "samples.json");
        auto samples = sRepo.findAll();
        if (samples.empty()) return;

        static const char* customers[] = {
            "Samsung Foundry", "SK Hynix", "LG Innotek",
            "DB HiTek", "TSMC Korea", "Hanwha Solutions"
        };
        static int quantities[] = {100, 150, 200, 50, 300, 400, 80, 250, 120, 60};
        static OrderStatus statuses[] = {
            OrderStatus::RESERVED, OrderStatus::CONFIRMED, OrderStatus::PRODUCING,
            OrderStatus::RELEASE, OrderStatus::RESERVED, OrderStatus::CONFIRMED,
            OrderStatus::RELEASE, OrderStatus::RELEASE, OrderStatus::CONFIRMED,
            OrderStatus::RESERVED
        };

        int existing = static_cast<int>(repo.findAll().size());
        for (int i = 0; i < count && i < 10; ++i) {
            int seq = existing + i + 1;
            std::string orderId = Order::generateOrderId("20260612", seq);
            if (repo.findById(orderId).has_value()) continue;

            const auto& s = samples[i % samples.size()];
            Order o(orderId, s.id, customers[i % 6], quantities[i % 10]);

            OrderStatus target = statuses[i % 10];
            if (target == OrderStatus::CONFIRMED || target == OrderStatus::PRODUCING ||
                target == OrderStatus::REJECTED)
                o.changeStatus(target);
            else if (target == OrderStatus::RELEASE) {
                o.changeStatus(OrderStatus::CONFIRMED);
                o.changeStatus(OrderStatus::RELEASE);
            }
            repo.save(o);
        }
    }

private:
    std::string dataDir_;
};
