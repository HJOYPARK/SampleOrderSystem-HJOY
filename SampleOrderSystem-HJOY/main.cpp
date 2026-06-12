#define NOMINMAX
#include <windows.h>
#include "Repository/SampleRepository.h"
#include "Repository/OrderRepository.h"
#include "Model/ProductionQueue.h"
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

int main() {
    // Set console output to UTF-8 so Korean strings display correctly
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // Disable sync for faster I/O
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // Ensure data directory exists
    std::filesystem::create_directories("data");

    // Repository initialization
    SampleRepository sampleRepo("data/samples.json");
    OrderRepository  orderRepo("data/orders.json");
    ProductionQueue  productionQueue;

    // Generate sample data if empty
    if (sampleRepo.findAll().empty()) {
        DummyDataGenerator gen("data/");
        gen.generateSamples(5);
    }

    // Controller initialization (dependency injection)
    SampleController     sampleCtrl(&sampleRepo);
    OrderController      orderCtrl(&sampleRepo, &orderRepo, &productionQueue);
    ProductionController prodCtrl(&productionQueue, &sampleRepo, &orderRepo);
    MonitoringController monCtrl(&orderRepo, &sampleRepo);
    ReleaseController    releaseCtrl(&orderRepo, &sampleRepo);

    // View initialization
    MainView       mainView;
    SampleView     sampleView(sampleCtrl);
    OrderView      orderView(orderCtrl, sampleRepo);
    ApprovalView   approvalView(orderCtrl, sampleRepo);
    MonitoringView monitoringView(monCtrl);
    ProductionView productionView(prodCtrl, sampleRepo, orderRepo);
    ReleaseView    releaseView(releaseCtrl, sampleRepo);

    // Main loop
    while (true) {
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
