#pragma once
#include "ViewHelper.h"
#include "Controller/OrderController.h"
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/ProductionJob.h"
#include <iostream>
#include <iomanip>

class OrderView {
public:
    OrderView(OrderController& ctrl, IRepository<Sample>& sampleRepo)
        : ctrl_(ctrl), sampleRepo_(sampleRepo) {}

    void run() {
        printSeparator();
        std::cout << "[2] 시료 주문\n\n";

        std::string sampleId   = getString("시료 ID    > ");
        auto sampleOpt = sampleRepo_.findById(sampleId);
        if (!sampleOpt.has_value()) {
            std::cout << "\n오류: 등록되지 않은 시료 ID입니다.\n";
            return;
        }

        std::string customer = getString("고객명     > ");
        int qty = getInt("주문 수량  > ");
        if (qty <= 0) { std::cout << "오류: 수량은 양수여야 합니다.\n"; return; }

        std::cout << "\n입력 내용 확인\n";
        std::cout << "시료       " << sampleOpt->name << "  (" << sampleId << ")\n";
        std::cout << "고객       " << customer << "\n";
        std::cout << "수량       " << qty << " ea\n\n";
        std::cout << "[Y] 예약 접수   [N] 취소\n";
        char c = getChar("선택 > ");
        if (c != 'Y') { std::cout << "접수 취소.\n"; return; }

        auto orderId = ctrl_.placeOrder(sampleId, customer, qty);
        if (orderId.empty()) { std::cout << "오류: 주문 접수 실패.\n"; return; }

        std::cout << "\n예약 접수 완료.\n\n";
        std::cout << "주문번호   " << orderId << "\n";
        std::cout << "현재 상태  RESERVED\n\n";
        std::cout << "※ 재고 확인은 [3] 승인 메뉴에서 직접 진행하세요.\n";
    }

private:
    OrderController&     ctrl_;
    IRepository<Sample>& sampleRepo_;
};
