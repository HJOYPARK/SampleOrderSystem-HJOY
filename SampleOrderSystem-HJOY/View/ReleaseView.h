#pragma once
#include "ViewHelper.h"
#include "Controller/ReleaseController.h"
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include <iostream>
#include <iomanip>

class ReleaseView {
public:
    ReleaseView(ReleaseController& ctrl, IRepository<Sample>& sampleRepo)
        : ctrl_(ctrl), sampleRepo_(sampleRepo) {}

    void run() {
        while (true) {
            printSeparator();
            std::cout << "[6] 출고 처리\n\n";

            auto releasable = ctrl_.getReleasableOrders();
            if (releasable.empty()) {
                std::cout << "출고 가능한 주문이 없습니다 (CONFIRMED 상태 없음)\n";
                getString("Enter를 누르면 위로...");
                break;
            }

            std::cout << "출고 가능 주문  (CONFIRMED)\n\n";
            std::cout << std::left
                      << std::setw(6)  << "번호"
                      << std::setw(20) << "주문번호"
                      << std::setw(14) << "고객"
                      << std::setw(22) << "시료"
                      << "수량\n";

            for (int i = 0; i < static_cast<int>(releasable.size()); ++i) {
                auto& o = releasable[i];
                auto sOpt = sampleRepo_.findById(o.sampleId);
                std::string sname = sOpt.has_value() ? sOpt->name : o.sampleId;
                std::cout << "[" << (i+1) << "]"
                          << std::setw(3)  << ""
                          << std::setw(20) << o.orderId
                          << std::setw(14) << o.customerName
                          << std::setw(22) << sname
                          << o.quantity << " ea\n";
            }

            std::cout << "\n[0] 위로\n";
            int choice = getInt("출고할 번호 > ");
            if (choice == 0) break;
            if (choice < 1 || choice > static_cast<int>(releasable.size())) continue;

            auto& selected = releasable[choice - 1];
            if (ctrl_.releaseOrder(selected.orderId)) {
                std::cout << "\n출고 처리 완료.\n\n";
                std::cout << "주문번호   " << selected.orderId << "\n";
                std::cout << "출고수량   " << selected.quantity << " ea\n";
                std::cout << "처리일시   " << currentDateTimeString() << "\n";
                std::cout << "상태       CONFIRMED -> RELEASE\n";
            } else {
                std::cout << "\n출고 처리 실패.\n";
            }
            getString("\nEnter를 누르면 계속...");
        }
    }

private:
    ReleaseController&   ctrl_;
    IRepository<Sample>& sampleRepo_;
};
