#pragma once
#include "ViewHelper.h"
#include "Controller/ProductionController.h"
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Model/Order.h"
#include <iostream>
#include <iomanip>
#include <cmath>

class ProductionView {
public:
    ProductionView(ProductionController& ctrl,
                   IRepository<Sample>& sampleRepo,
                   IRepository<Order>& orderRepo)
        : ctrl_(ctrl), sampleRepo_(sampleRepo), orderRepo_(orderRepo) {}

    void run() {
        while (true) {
            ctrl_.syncProduction();

            printSeparator();
            std::cout << "[5] 생산라인 조회   FIFO 방식\n\n";
            std::cout << "생산라인 1개 (단일 라인)\n\n";

            auto current = ctrl_.getCurrentJob();
            if (current.has_value()) {
                auto& job = *current;
                auto sOpt = sampleRepo_.findById(job.sampleId);
                std::string sname = sOpt.has_value() ? sOpt->name : job.sampleId;

                // 실시간 진행률 계산
                int    produced  = ctrl_.getLiveProducedCount();
                double progress  = ctrl_.getCurrentJobProgress();
                int    pct       = static_cast<int>(progress * 100);
                double remainMin = job.avgProductionTime * (job.actualProduction - produced);

                std::cout << "현재 처리 중  [RUNNING]\n\n";
                std::cout << "  주문번호  " << job.orderId << "   시료  " << sname << "\n";

                double yr = sOpt.has_value() ? sOpt->yieldRate : 0.0;
                std::cout << "  부족분    " << job.shortage << " ea"
                          << "   실생산량  " << job.actualProduction << " ea"
                          << "   (수율 " << std::fixed << std::setprecision(2) << yr
                          << " / " << std::fixed << std::setprecision(1)
                          << job.totalProductionTime << " min)\n";

                std::cout << "  생산완료  " << produced << " / " << job.actualProduction << " ea"
                          << "   (개당 " << job.avgProductionTime << " min)\n";

                std::cout << "  진행      " << makeBar(progress) << "  " << pct << "%";
                if (progress < 1.0)
                    std::cout << "   잔여 " << std::fixed << std::setprecision(1) << remainMin << " min";
                else
                    std::cout << "   완료 처리 중...";
                std::cout << "\n\n";
            } else {
                std::cout << "현재 처리 중인 생산 작업 없음\n\n";
            }

            auto waiting = ctrl_.getWaitingJobs();
            std::cout << "대기 중인 주문  (FIFO 순)  " << waiting.size() << "건\n\n";
            if (!waiting.empty()) {
                std::cout << padRight("순서", 6)
                          << padRight("주문번호", 20)
                          << padRight("시료", 22)
                          << padRight("부족분", 10)
                          << "실생산량\n";
                for (int i = 0; i < static_cast<int>(waiting.size()); ++i) {
                    auto& job = waiting[i];
                    auto sOpt = sampleRepo_.findById(job.sampleId);
                    std::string sname = sOpt.has_value() ? sOpt->name : job.sampleId;
                    std::cout << padRight(std::to_string(i+1), 6)
                              << padRight(job.orderId, 20)
                              << padRight(sname, 22)
                              << padRight(std::to_string(job.shortage) + " ea", 10)
                              << job.actualProduction << " ea\n";
                }
            }

            std::cout << "\n* 실생산량 = ceil(부족분 / (수율 * 0.9))\n";
            std::cout << "* 선입선출(FIFO) 방식 / 실제 시간 기반 자동 완료\n\n";

            getString("[0] 위로 (Enter) > ");
            break;
        }
    }

private:
    ProductionController& ctrl_;
    IRepository<Sample>&  sampleRepo_;
    IRepository<Order>&   orderRepo_;
};
