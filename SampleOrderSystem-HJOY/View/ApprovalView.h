#pragma once
#include "ViewHelper.h"
#include "Controller/OrderController.h"
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/ProductionJob.h"
#include <iostream>
#include <iomanip>
#include <cmath>

class ApprovalView {
public:
    ApprovalView(OrderController& ctrl, IRepository<Sample>& sampleRepo)
        : ctrl_(ctrl), sampleRepo_(sampleRepo) {}

    void run() {
        while (true) {
            printSeparator();
            std::cout << "[3] 주문 승인/거절\n\n";
            auto pending = ctrl_.getPendingOrders();
            if (pending.empty()) {
                std::cout << "승인 대기 중인 주문이 없습니다.\n";
                getString("Enter를 누르면 위로...");
                break;
            }

            std::cout << "승인 대기 중인 예약 목록  (RESERVED)\n\n";
            std::cout << std::left
                      << std::setw(6)  << "번호"
                      << std::setw(20) << "주문번호"
                      << std::setw(18) << "고객"
                      << std::setw(22) << "시료"
                      << std::setw(10) << "수량"
                      << "상태\n";

            for (int i = 0; i < static_cast<int>(pending.size()); ++i) {
                auto& o = pending[i];
                auto sOpt = sampleRepo_.findById(o.sampleId);
                std::string sname = sOpt.has_value() ? sOpt->name : o.sampleId;
                std::cout << "[" << (i+1) << "]"
                          << std::setw(3) << ""
                          << std::setw(20) << o.orderId
                          << std::setw(18) << o.customerName
                          << std::setw(22) << sname
                          << std::setw(10) << (std::to_string(o.quantity) + " ea")
                          << "RESERVED\n";
            }

            std::cout << "\n[0] 위로\n";
            int choice = getInt("승인할 번호 > ");
            if (choice == 0) break;
            if (choice < 1 || choice > static_cast<int>(pending.size())) continue;

            auto& selected = pending[choice - 1];
            auto sampleOpt = sampleRepo_.findById(selected.sampleId);
            if (!sampleOpt.has_value()) { std::cout << "시료 정보 오류.\n"; continue; }

            auto& sample = *sampleOpt;
            int shortage = ProductionJob::calcShortage(selected.quantity, sample.stock);

            std::cout << "\n재고 확인 중...\n\n";
            std::cout << "시료       " << sample.name << "   현재 재고  " << sample.stock << " ea\n";
            std::cout << "주문 수량  " << selected.quantity << " ea";

            if (shortage > 0) {
                int actualProd = static_cast<int>(
                    std::ceil(static_cast<double>(shortage) / (sample.yieldRate * 0.9)));
                double totalTime = sample.avgProductionTime * actualProd;
                std::cout << "               부족분     " << shortage << " ea  <- 이 수량만큼 생산\n\n";
                std::cout << "재고 부족.  부족분 " << shortage << " ea 승인하시겠습니까?"
                          << "  (실생산량 " << actualProd << " ea / "
                          << static_cast<int>(totalTime) << " min)\n\n";
                std::cout << "[Y] 승인   [N] 주문 거절\n";
                char c = getChar("선택 > ");
                if (c == 'Y') {
                    auto result = ctrl_.approveOrder(selected.orderId);
                    std::cout << "\n승인 완료.\n\n";
                    std::cout << "상태 변경   RESERVED -> PRODUCING\n";
                    std::cout << "주문번호    " << selected.orderId << "\n";
                } else {
                    ctrl_.rejectOrder(selected.orderId);
                    std::cout << "\n거절 완료.  주문번호 " << selected.orderId << "  REJECTED\n";
                }
            } else {
                std::cout << "\n재고 충분.  즉시 승인합니다.\n";
                ctrl_.approveOrder(selected.orderId);
                std::cout << "\n승인 완료.\n\n";
                std::cout << "상태 변경   RESERVED -> CONFIRMED\n";
                std::cout << "주문번호    " << selected.orderId << "\n";
            }
            getString("\nEnter를 누르면 계속...");
        }
    }

private:
    OrderController&     ctrl_;
    IRepository<Sample>& sampleRepo_;
};
