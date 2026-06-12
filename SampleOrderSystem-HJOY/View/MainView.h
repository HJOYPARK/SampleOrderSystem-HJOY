#pragma once
#include "ViewHelper.h"
#include "Model/Sample.h"
#include "Model/Order.h"
#include "Model/IProductionQueue.h"
#include "Repository/IRepository.h"
#include <vector>
#include <iostream>

class MainView {
public:
    void display(IRepository<Sample>& sampleRepo,
                 IRepository<Order>& orderRepo,
                 IProductionQueue& queue) {
        auto samples = sampleRepo.findAll();
        auto orders  = orderRepo.findAll();

        int totalStock = 0;
        for (auto& s : samples) totalStock += s.stock;
        int queueSize = static_cast<int>(queue.getAllJobs().size());

        printSeparator();
        std::cout << "반도체 시료 생산주문관리 시스템\n";
        printSeparator();
        std::cout << "시스템 현황  " << currentDateTimeString() << "\n\n";
        std::cout << "등록 시료  " << samples.size() << "종"
                  << "       총 재고  " << totalStock << " ea\n";
        std::cout << "전체 주문  " << orders.size() << "건"
                  << "       생산라인 " << queueSize << "건 대기\n\n";
        std::cout << "[1] 시료 관리          [2] 시료 주문\n";
        std::cout << "[3] 주문 승인/거절     [4] 모니터링\n";
        std::cout << "[5] 생산라인 조회      [6] 출고 처리\n";
        std::cout << "[0] 종료\n\n";
    }

    int getMenuChoice() {
        return getInt("선택 > ");
    }
};
