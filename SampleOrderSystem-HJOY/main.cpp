#define NOMINMAX
#include <windows.h>
#include "Repository/SampleRepository.h"
#include "Repository/OrderRepository.h"
#include "Model/ProductionQueue.h"
#include "Model/ProductionJob.h"
#include "Controller/SampleController.h"
#include "Controller/OrderController.h"
#include "Controller/ProductionController.h"
#include "Controller/MonitoringController.h"
#include "Controller/ReleaseController.h"
#include "View/MainView.h"
#include "View/SampleView.h"
#include "View/OrderView.h"
#include "View/ApprovalView.h"
#include "View/MonitoringView.h"
#include "View/ProductionView.h"
#include "View/ReleaseView.h"
#include "Tools/DummyDataGenerator.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <vector>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::filesystem::create_directories("data");

    SampleRepository sampleRepo("data/samples.json");
    OrderRepository  orderRepo("data/orders.json");
    ProductionQueue  productionQueue;

    if (sampleRepo.findAll().empty()) {
        DummyDataGenerator gen("data/");
        gen.generateSamples(5);
    }

    // PRODUCING 상태 주문으로 생산 큐 재구성 (앱 재시작 시 이어서 진행)
    {
        auto allOrders = orderRepo.findAll();
        std::vector<Order> producing;
        for (auto& o : allOrders)
            if (o.status == OrderStatus::PRODUCING && o.shortage > 0)
                producing.push_back(o);
        std::sort(producing.begin(), producing.end(),
                  [](const Order& a, const Order& b) {
                      return a.productionStartTime < b.productionStartTime;
                  });
        for (auto& o : producing) {
            auto sOpt = sampleRepo.findById(o.sampleId);
            if (!sOpt.has_value()) continue;
            productionQueue.enqueue(
                ProductionJob(o.orderId, o.sampleId, o.shortage,
                              sOpt->yieldRate, sOpt->avgProductionTime));
        }
    }

    SampleController     sampleCtrl(&sampleRepo);
    OrderController      orderCtrl(&sampleRepo, &orderRepo, &productionQueue);
    ProductionController prodCtrl(&productionQueue, &sampleRepo, &orderRepo);
    MonitoringController monCtrl(&orderRepo, &sampleRepo);
    ReleaseController    releaseCtrl(&orderRepo, &sampleRepo);

    MainView       mainView;
    SampleView     sampleView(sampleCtrl, prodCtrl);
    OrderView      orderView(orderCtrl, sampleRepo);
    ApprovalView   approvalView(orderCtrl, sampleRepo);
    MonitoringView monitoringView(monCtrl, prodCtrl);
    ProductionView productionView(prodCtrl, sampleRepo, orderRepo);
    ReleaseView    releaseView(releaseCtrl, sampleRepo);

    while (true) {
        prodCtrl.syncProduction();
        mainView.display(sampleRepo, orderRepo, productionQueue);
        int choice = mainView.getMenuChoice();

        switch (choice) {
        case 0:
            std::cout << "\n시스템을 종료합니다.\n";
            return 0;
        case 1: sampleView.run();     break;
        case 2: orderView.run();      break;
        case 3: approvalView.run();   break;
        case 4: monitoringView.run(); break;
        case 5: productionView.run(); break;
        case 6: releaseView.run();    break;
        default:
            std::cout << "잘못된 선택입니다.\n";
        }
    }
}
