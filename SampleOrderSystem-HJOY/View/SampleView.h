#pragma once
#include "ViewHelper.h"
#include "Controller/SampleController.h"
#include <vector>
#include <iostream>
#include <iomanip>

class SampleView {
public:
    explicit SampleView(SampleController& ctrl) : ctrl_(ctrl) {}

    void run() {
        while (true) {
            printSeparator();
            std::cout << "[1] 시료 관리\n\n";
            std::cout << "[1] 시료 등록   [2] 시료 목록   [3] 시료 검색   [0] 위로\n";
            int choice = getInt("선택 > ");
            if (choice == 0) break;
            if (choice == 1) registerSample();
            else if (choice == 2) listSamples();
            else if (choice == 3) searchSamples();
        }
    }

private:
    SampleController& ctrl_;

    void listSamples(const std::vector<Sample>& samples, int page) {
        const int PAGE_SIZE = 5;
        int total = static_cast<int>(samples.size());
        int start = page * PAGE_SIZE;
        int end   = std::min(start + PAGE_SIZE, total);

        std::cout << "\n등록 시료 목록  (총 " << total << "종)\n\n";
        std::cout << padRight("ID", 10)
                  << padRight("시료명", 22)
                  << padRight("평균 생산시간", 16)
                  << padRight("수율", 8)
                  << "현재 재고\n"
                  << std::string(70, '-') << "\n";

        for (int i = start; i < end; ++i) {
            const auto& s = samples[i];
            std::ostringstream aptStr, yrStr;
            aptStr << std::fixed << std::setprecision(1) << s.avgProductionTime << " min/ea";
            yrStr  << std::fixed << std::setprecision(2) << s.yieldRate;
            std::cout << padRight(s.id,   10)
                      << padRight(s.name, 22)
                      << padRight(aptStr.str(), 16)
                      << padRight(yrStr.str(),  8)
                      << s.stock << " ea\n";
        }
        if (end < total)
            std::cout << "...외 " << (total - end) << "종   [N] 다음페이지\n";
    }

    void listSamples() {
        auto all = ctrl_.listSamples();
        int page = 0;
        while (true) {
            listSamples(all, page);
            std::cout << "\n[N] 다음   [P] 이전   [0] 위로\n";
            char c = getChar("선택 > ");
            if (c == '0') break;
            else if (c == 'N' && (page + 1) * 5 < static_cast<int>(all.size())) ++page;
            else if (c == 'P' && page > 0) --page;
        }
    }

    void registerSample() {
        printSeparator();
        std::cout << "시료 등록\n\n";
        std::string id   = getString("시료 ID       > ");
        std::string name = getString("시료명        > ");
        double apt = 0, yr = 0;
        int stock = 0;
        try {
            apt   = std::stod(getString("평균 생산시간 (min/ea) > "));
            yr    = std::stod(getString("수율 (0.0~1.0)        > "));
            stock = std::stoi(getString("현재 재고 (ea)        > "));
        } catch (...) {
            std::cout << "입력 오류.\n";
            return;
        }
        if (ctrl_.registerSample(id, name, apt, yr, stock))
            std::cout << "\n시료 등록 완료.  ID: " << id << "\n";
        else
            std::cout << "\n오류: 중복 ID 또는 유효하지 않은 값.\n";
    }

    void searchSamples() {
        std::string kw = getString("검색어 > ");
        auto result = ctrl_.searchByName(kw);
        std::cout << "\n검색 결과  " << result.size() << "건\n\n";
        for (auto& s : result)
            std::cout << s.id << "  " << s.name << "  재고 " << s.stock << " ea\n";
    }
};
