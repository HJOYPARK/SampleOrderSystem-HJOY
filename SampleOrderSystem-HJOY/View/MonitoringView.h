#pragma once
#include "ViewHelper.h"
#include "Controller/MonitoringController.h"
#include "Controller/ProductionController.h"
#include <iostream>
#include <iomanip>
#include <map>

class MonitoringView {
public:
    MonitoringView(MonitoringController& ctrl, ProductionController& prodCtrl)
        : ctrl_(ctrl), prodCtrl_(prodCtrl) {}

    void run() {
        while (true) {
            prodCtrl_.syncProduction();
            printSeparator();
            std::cout << "[4] 모니터링   " << currentDateTimeString() << "\n\n";
            std::cout << "[1] 주문량 확인   [2] 재고량 확인   [0] 위로\n";
            int choice = getInt("선택 > ");
            if (choice == 0) break;
            if (choice == 1) showOrderCounts();
            else if (choice == 2) showStockStatuses();
        }
    }

private:
    MonitoringController& ctrl_;
    ProductionController& prodCtrl_;

    void showOrderCounts() {
        auto counts = ctrl_.getOrderCounts();
        std::cout << "\n상태별 주문 현황\n";
        auto print = [&](OrderStatus s, const char* label, const char* note = "") {
            int cnt = counts.count(s) ? counts.at(s) : 0;
            std::cout << std::left << std::setw(14) << label << cnt << "건  " << note << "\n";
        };
        print(OrderStatus::RESERVED,  "RESERVED");
        print(OrderStatus::CONFIRMED, "CONFIRMED");
        print(OrderStatus::PRODUCING, "PRODUCING", "<- 생산라인 대기");
        print(OrderStatus::RELEASE,   "RELEASE");
        getString("\nEnter를 누르면 위로...");
    }

    void showStockStatuses() {
        auto statuses = ctrl_.getStockStatuses();
        std::cout << "\n재고 현황\n\n";
        std::cout << padRight("시료명", 24)
                  << padRight("재고", 12)
                  << padRight("상태", 8)
                  << "잔여율\n";

        for (auto& [id, status] : statuses) {
            std::string label;
            if (status == StockStatus::SUFFICIENT) label = "여유";
            else if (status == StockStatus::SHORTAGE) label = "부족";
            else label = "고갈";
            std::cout << padRight(id, 24)
                      << padRight("-", 12)
                      << padRight(label, 8)
                      << "\n";
        }
        getString("\nEnter를 누르면 위로...");
    }
};
