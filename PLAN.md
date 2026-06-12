# 반도체 시료 생산주문관리 시스템 - 개발 계획

## TDD 원칙 (SKILL.md 기반)

```
철칙: 테스트 없이 프로덕션 코드 작성 금지 (NO PRODUCTION CODE WITHOUT A FAILING TEST FIRST)
```

### RED-GREEN-REVIEW 사이클
| 단계 | 행동 | 확인 |
|------|------|------|
| **RED** | 실패하는 테스트 1개 작성 | 올바른 이유로 실패하는지 빌드로 확인 |
| **GREEN** | 테스트를 통과하는 최소한의 코드 작성 | 모든 테스트 통과 확인 |
| **REVIEW** | 중복 제거, 이름 개선, 헬퍼 추출 | 리뷰 후에도 GREEN 유지 확인 |

### gmock 사용 원칙
- Controller 레이어: Repository 인터페이스를 Mock으로 주입 (의존성 역전)
- Model/Repository 레이어: 실제 코드로 테스트 (Mock 사용 금지)
- Mock은 반드시 인터페이스 기반으로만 작성

### 빌드 및 테스트 실행 명령
```powershell
# 전체 빌드
msbuild SampleOrderSystem-HJOY.slnx /p:Configuration=Debug /p:Platform=x64

# 테스트 실행 (테스트 프로젝트 빌드 후)
.\x64\Debug\SampleOrderSystemTest.exe

# 특정 테스트만 실행
.\x64\Debug\SampleOrderSystemTest.exe --gtest_filter=SampleTest.*
```

---

## Phase 0: 개발 환경 구축 (테스트 프레임워크 설정)

> 목표: Google Test + Google Mock을 프로젝트에 통합하고 빈 테스트가 통과하는 것을 확인

### Step 0-1: 테스트 프로젝트 생성
- Visual Studio에서 `SampleOrderSystemTest` 콘솔 프로젝트 추가 (솔루션에 추가)
- NuGet으로 `Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn` 패키지 설치
- 또는 vcpkg: `vcpkg install gtest:x64-windows`
- 테스트 프로젝트가 메인 프로젝트를 참조하도록 설정

### Step 0-2: 기본 테스트 파일 구성
```
SampleOrderSystemTest/
├── main_test.cpp        # gtest main 진입점
├── SampleTest.cpp
├── OrderTest.cpp
├── ProductionJobTest.cpp
├── SampleRepositoryTest.cpp
├── OrderRepositoryTest.cpp
├── SampleControllerTest.cpp
├── OrderControllerTest.cpp
├── ProductionControllerTest.cpp
├── MonitoringControllerTest.cpp
└── ReleaseControllerTest.cpp
```

### Step 0-3: 첫 번째 더미 테스트로 환경 확인
```cpp
// main_test.cpp
#include <gtest/gtest.h>
TEST(EnvironmentTest, GoogleTestWorks) {
    EXPECT_EQ(1, 1);
}
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```
- [ ] RED → GREEN 확인: 빌드 성공 + PASS 출력 확인

---

## Phase 1: 도메인 모델 - Sample

> 목표: Sample 구조체와 유효성 규칙을 TDD로 구현

### Step 1-1: Sample 생성 기본 동작
**RED** — 테스트 작성
```cpp
TEST(SampleTest, CanCreateSampleWithValidFields) {
    Sample s("S-001", "실리콘 웨이퍼-8인치", 0.5, 0.92, 480);
    EXPECT_EQ(s.id, "S-001");
    EXPECT_EQ(s.name, "실리콘 웨이퍼-8인치");
    EXPECT_DOUBLE_EQ(s.avgProductionTime, 0.5);
    EXPECT_DOUBLE_EQ(s.yieldRate, 0.92);
    EXPECT_EQ(s.stock, 480);
}
```
**GREEN** — `Model/Sample.h` + `Sample.cpp` 최소 구현
**REVIEW** — 필드명, 생성자 파라미터 순서 점검

### Step 1-2: 수율 유효성 검사
**RED**
```cpp
TEST(SampleTest, YieldRateMustBeBetweenZeroAndOne) {
    EXPECT_THROW(Sample("S-002", "test", 0.5, 1.5, 0), std::invalid_argument);
    EXPECT_THROW(Sample("S-002", "test", 0.5, -0.1, 0), std::invalid_argument);
}
```
**GREEN** — 생성자에서 yieldRate 범위 검사 추가
**REVIEW** — 예외 메시지 명확성 점검

### Step 1-3: 평균 생산시간 유효성 검사
**RED**
```cpp
TEST(SampleTest, AvgProductionTimeMustBePositive) {
    EXPECT_THROW(Sample("S-003", "test", 0.0, 0.9, 0), std::invalid_argument);
    EXPECT_THROW(Sample("S-003", "test", -1.0, 0.9, 0), std::invalid_argument);
}
```
**GREEN** — avgProductionTime > 0 검사 추가
**REVIEW**

### Step 1-4: 재고 음수 방지
**RED**
```cpp
TEST(SampleTest, StockCannotBeNegative) {
    EXPECT_THROW(Sample("S-004", "test", 0.5, 0.9, -1), std::invalid_argument);
}
```
**GREEN**
**REVIEW**

---

## Phase 2: 도메인 모델 - Order

> 목표: Order 구조체, OrderStatus, 주문번호 형식을 TDD로 구현

### Step 2-1: Order 생성 기본 동작
**RED**
```cpp
TEST(OrderTest, CanCreateOrderWithReservedStatus) {
    Order o("ORD-20260416-0001", "S-001", "삼성전자", 200);
    EXPECT_EQ(o.orderId, "ORD-20260416-0001");
    EXPECT_EQ(o.sampleId, "S-001");
    EXPECT_EQ(o.customerName, "삼성전자");
    EXPECT_EQ(o.quantity, 200);
    EXPECT_EQ(o.status, OrderStatus::RESERVED);
}
```
**GREEN** — `Model/Order.h` + `Order.cpp` + `OrderStatus` enum 구현
**REVIEW**

### Step 2-2: 주문 수량 유효성 검사
**RED**
```cpp
TEST(OrderTest, QuantityMustBePositive) {
    EXPECT_THROW(Order("ORD-20260416-0002", "S-001", "고객", 0), std::invalid_argument);
    EXPECT_THROW(Order("ORD-20260416-0002", "S-001", "고객", -5), std::invalid_argument);
}
```
**GREEN**
**REVIEW**

### Step 2-3: 주문번호 형식 생성기
**RED**
```cpp
TEST(OrderTest, GenerateOrderIdWithCorrectFormat) {
    // 형식: ORD-YYYYMMDD-NNNN
    std::string id = Order::generateOrderId("20260416", 43);
    EXPECT_EQ(id, "ORD-20260416-0043");
}
TEST(OrderTest, GenerateOrderIdPadsSequenceToFourDigits) {
    std::string id = Order::generateOrderId("20260416", 1);
    EXPECT_EQ(id, "ORD-20260416-0001");
}
```
**GREEN** — static `generateOrderId()` 메서드 구현
**REVIEW**

### Step 2-4: 상태 전환 유효성 검사
**RED**
```cpp
TEST(OrderTest, CanTransitionFromReservedToConfirmed) {
    Order o("ORD-20260416-0001", "S-001", "고객", 100);
    EXPECT_NO_THROW(o.changeStatus(OrderStatus::CONFIRMED));
    EXPECT_EQ(o.status, OrderStatus::CONFIRMED);
}
TEST(OrderTest, CannotTransitionFromReleaseToAnyOtherStatus) {
    Order o("ORD-20260416-0001", "S-001", "고객", 100);
    o.changeStatus(OrderStatus::CONFIRMED);
    o.changeStatus(OrderStatus::RELEASE);
    EXPECT_THROW(o.changeStatus(OrderStatus::RESERVED), std::logic_error);
}
```
**GREEN** — `changeStatus()` 메서드 + 허용 전환 규칙 구현
**REVIEW**

---

## Phase 3: 도메인 모델 - ProductionJob (생산량 계산)

> 목표: 실 생산량·총 생산시간 계산 공식을 TDD로 검증

### Step 3-1: 실 생산량 계산
**RED**
```cpp
TEST(ProductionJobTest, CalculatesActualProductionWithYieldAndBuffer) {
    // 실 생산량 = ceil(부족분 / (수율 * 0.9))
    // 부족분 170, 수율 0.92 → ceil(170 / 0.828) = ceil(205.3) = 206
    ProductionJob job("ORD-001", "S-003", 170, 0.92, 0.8);
    EXPECT_EQ(job.actualProduction, 206);
}
```
**GREEN** — `Model/ProductionJob.h` + 계산 로직 구현
**REVIEW**

### Step 3-2: 총 생산시간 계산
**RED**
```cpp
TEST(ProductionJobTest, CalculatesTotalProductionTime) {
    // 총 생산시간 = 평균 생산시간 * 실 생산량
    // 0.8 min/ea * 206 ea = 164.8 min
    ProductionJob job("ORD-001", "S-003", 170, 0.92, 0.8);
    EXPECT_DOUBLE_EQ(job.totalProductionTime, 164.8);
}
```
**GREEN**
**REVIEW**

### Step 3-3: 부족분이 0이면 생산 불필요
**RED**
```cpp
TEST(ProductionJobTest, ZeroShortageProducesZeroActualProduction) {
    EXPECT_THROW(ProductionJob("ORD-001", "S-001", 0, 0.92, 0.5), std::invalid_argument);
}
```
**GREEN**
**REVIEW**

### Step 3-4: 재고 충분 여부 판단
**RED**
```cpp
TEST(ProductionJobTest, ShortageCalculatedAsOrderQuantityMinusCurrentStock) {
    // 주문량 200, 현재 재고 30 → 부족분 170
    int shortage = ProductionJob::calcShortage(200, 30);
    EXPECT_EQ(shortage, 170);
}
TEST(ProductionJobTest, NoShortageWhenStockCoversOrder) {
    int shortage = ProductionJob::calcShortage(100, 150);
    EXPECT_EQ(shortage, 0);
}
```
**GREEN** — static `calcShortage()` 구현
**REVIEW**

---

## Phase 4: Repository 레이어 - 인터페이스 및 JSON 직렬화

> 목표: IRepository 인터페이스 정의 및 JSON 파일 기반 영속성 구현

### Step 4-1: IRepository 인터페이스 정의
- 구현 코드 없음, 인터페이스(순수 가상 클래스)만 작성
```cpp
// Repository/IRepository.h
template<typename T>
class IRepository {
public:
    virtual void save(const T& entity) = 0;
    virtual std::optional<T> findById(const std::string& id) = 0;
    virtual std::vector<T> findAll() = 0;
    virtual void update(const T& entity) = 0;
    virtual void remove(const std::string& id) = 0;
    virtual ~IRepository() = default;
};
```

### Step 4-2: JsonSerializer - Sample 직렬화
**RED**
```cpp
TEST(JsonSerializerTest, SerializesSampleToJsonString) {
    Sample s("S-001", "실리콘 웨이퍼-8인치", 0.5, 0.92, 480);
    std::string json = JsonSerializer::toJson(s);
    EXPECT_NE(json.find("\"id\":\"S-001\""), std::string::npos);
    EXPECT_NE(json.find("\"stock\":480"), std::string::npos);
}
```
**GREEN** — `Repository/JsonSerializer.h/.cpp` 구현 (표준 C++ 문자열 조작)
**REVIEW**

### Step 4-3: JsonSerializer - Sample 역직렬화
**RED**
```cpp
TEST(JsonSerializerTest, DeserializesSampleFromJsonString) {
    std::string json = R"({"id":"S-001","name":"실리콘 웨이퍼-8인치","avgProductionTime":0.5,"yieldRate":0.92,"stock":480})";
    Sample s = JsonSerializer::sampleFromJson(json);
    EXPECT_EQ(s.id, "S-001");
    EXPECT_EQ(s.stock, 480);
}
```
**GREEN**
**REVIEW**

### Step 4-4: JsonSerializer - Order 직렬화/역직렬화
**RED**
```cpp
TEST(JsonSerializerTest, SerializesAndDeserializesOrderRoundTrip) {
    Order original("ORD-20260416-0001", "S-001", "삼성전자", 200);
    std::string json = JsonSerializer::toJson(original);
    Order restored = JsonSerializer::orderFromJson(json);
    EXPECT_EQ(restored.orderId, original.orderId);
    EXPECT_EQ(restored.status, original.status);
}
```
**GREEN**
**REVIEW**

### Step 4-5: SampleRepository - CRUD
**RED** (실제 임시 파일 경로 사용)
```cpp
TEST(SampleRepositoryTest, SaveAndFindSampleById) {
    SampleRepository repo("test_samples.json");
    Sample s("S-001", "테스트 시료", 0.5, 0.9, 100);
    repo.save(s);
    auto found = repo.findById("S-001");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "테스트 시료");
}
TEST(SampleRepositoryTest, UpdateSampleStock) {
    SampleRepository repo("test_samples.json");
    Sample s("S-001", "테스트 시료", 0.5, 0.9, 100);
    repo.save(s);
    s.stock = 200;
    repo.update(s);
    auto found = repo.findById("S-001");
    EXPECT_EQ(found->stock, 200);
}
TEST(SampleRepositoryTest, FindAllReturnsSavedSamples) { ... }
TEST(SampleRepositoryTest, RemoveSampleById) { ... }
```
**GREEN** — `Repository/SampleRepository.h/.cpp` 구현
**REVIEW** — 테스트 후 임시 파일 정리 (`TearDown`)

### Step 4-6: OrderRepository - CRUD + 상태별 조회
**RED**
```cpp
TEST(OrderRepositoryTest, FindOrdersByStatus) {
    OrderRepository repo("test_orders.json");
    repo.save(Order("ORD-001", "S-001", "고객A", 100));
    repo.save(Order("ORD-002", "S-001", "고객B", 50));
    // ORD-002를 CONFIRMED로 변경 후 저장
    auto reserved = repo.findByStatus(OrderStatus::RESERVED);
    EXPECT_EQ(reserved.size(), 1);
    EXPECT_EQ(reserved[0].orderId, "ORD-001");
}
```
**GREEN** — `Repository/OrderRepository.h/.cpp` 구현
**REVIEW**

---

## Phase 5: SampleController - 시료 관리

> 목표: gmock으로 ISampleRepository를 주입하여 Controller 로직을 TDD로 검증

### Step 5-1: Mock 클래스 정의
```cpp
// SampleControllerTest.cpp
#include <gmock/gmock.h>
class MockSampleRepository : public IRepository<Sample> {
public:
    MOCK_METHOD(void, save, (const Sample&), (override));
    MOCK_METHOD(std::optional<Sample>, findById, (const std::string&), (override));
    MOCK_METHOD(std::vector<Sample>, findAll, (), (override));
    MOCK_METHOD(void, update, (const Sample&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};
```

### Step 5-2: 시료 등록
**RED**
```cpp
TEST(SampleControllerTest, RegisterSampleCallsRepositorySave) {
    MockSampleRepository mockRepo;
    SampleController controller(&mockRepo);
    EXPECT_CALL(mockRepo, findById("S-001")).WillOnce(Return(std::nullopt));
    EXPECT_CALL(mockRepo, save(testing::_)).Times(1);
    bool result = controller.registerSample("S-001", "실리콘 웨이퍼", 0.5, 0.92, 480);
    EXPECT_TRUE(result);
}
```
**GREEN** — `Controller/SampleController.h/.cpp` 구현
**REVIEW**

### Step 5-3: 중복 시료 ID 거부
**RED**
```cpp
TEST(SampleControllerTest, RejectsDuplicateSampleId) {
    MockSampleRepository mockRepo;
    SampleController controller(&mockRepo);
    Sample existing("S-001", "기존 시료", 0.5, 0.9, 100);
    EXPECT_CALL(mockRepo, findById("S-001")).WillOnce(Return(existing));
    bool result = controller.registerSample("S-001", "새 시료", 0.3, 0.8, 0);
    EXPECT_FALSE(result);
}
```
**GREEN**
**REVIEW**

### Step 5-4: 시료 목록 조회
**RED**
```cpp
TEST(SampleControllerTest, ListSamplesReturnsAllFromRepository) {
    MockSampleRepository mockRepo;
    SampleController controller(&mockRepo);
    std::vector<Sample> samples = {
        Sample("S-001", "시료A", 0.5, 0.92, 100),
        Sample("S-002", "시료B", 0.3, 0.78, 50)
    };
    EXPECT_CALL(mockRepo, findAll()).WillOnce(Return(samples));
    auto result = controller.listSamples();
    EXPECT_EQ(result.size(), 2);
}
```
**GREEN**
**REVIEW**

### Step 5-5: 시료 이름 검색
**RED**
```cpp
TEST(SampleControllerTest, SearchSamplesByNameReturnsMatching) {
    MockSampleRepository mockRepo;
    SampleController controller(&mockRepo);
    std::vector<Sample> all = {
        Sample("S-001", "실리콘 웨이퍼", 0.5, 0.92, 100),
        Sample("S-002", "GaN 에피택셀", 0.3, 0.78, 50)
    };
    EXPECT_CALL(mockRepo, findAll()).WillOnce(Return(all));
    auto result = controller.searchByName("실리콘");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].id, "S-001");
}
```
**GREEN**
**REVIEW**

---

## Phase 6: OrderController - 주문 접수

> 목표: 주문 접수 흐름(시료 확인 → RESERVED 생성)을 TDD로 구현

### Step 6-1: Mock 클래스 정의
```cpp
class MockOrderRepository : public IRepository<Order> {
public:
    MOCK_METHOD(void, save, (const Order&), (override));
    MOCK_METHOD(std::optional<Order>, findById, (const std::string&), (override));
    MOCK_METHOD(std::vector<Order>, findAll, (), (override));
    MOCK_METHOD(void, update, (const Order&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};
```

### Step 6-2: 정상 주문 접수 → RESERVED
**RED**
```cpp
TEST(OrderControllerTest, PlaceOrderCreatesReservedOrder) {
    MockSampleRepository mockSampleRepo;
    MockOrderRepository mockOrderRepo;
    OrderController controller(&mockSampleRepo, &mockOrderRepo);
    Sample s("S-001", "실리콘 웨이퍼", 0.5, 0.92, 480);
    EXPECT_CALL(mockSampleRepo, findById("S-001")).WillOnce(Return(s));
    EXPECT_CALL(mockOrderRepo, save(testing::_)).Times(1);
    auto orderId = controller.placeOrder("S-001", "삼성전자", 200);
    EXPECT_FALSE(orderId.empty());
    EXPECT_THAT(orderId, testing::StartsWith("ORD-"));
}
```
**GREEN** — `Controller/OrderController.h/.cpp` 구현
**REVIEW**

### Step 6-3: 미등록 시료 주문 거부
**RED**
```cpp
TEST(OrderControllerTest, RejectsOrderForUnregisteredSample) {
    MockSampleRepository mockSampleRepo;
    MockOrderRepository mockOrderRepo;
    OrderController controller(&mockSampleRepo, &mockOrderRepo);
    EXPECT_CALL(mockSampleRepo, findById("S-999")).WillOnce(Return(std::nullopt));
    EXPECT_CALL(mockOrderRepo, save(testing::_)).Times(0);
    auto orderId = controller.placeOrder("S-999", "고객", 100);
    EXPECT_TRUE(orderId.empty());
}
```
**GREEN**
**REVIEW**

---

## Phase 7: OrderController - 주문 승인/거절

> 목표: 재고 판단 → CONFIRMED/PRODUCING 분기, 거절 → REJECTED 를 TDD로 구현

### Step 7-1: Mock IProductionQueue 정의
```cpp
class MockProductionQueue {
public:
    MOCK_METHOD(void, enqueue, (const ProductionJob&), ());
    MOCK_METHOD(std::optional<ProductionJob>, front, (), (const));
    MOCK_METHOD(void, dequeue, (), ());
    MOCK_METHOD(std::vector<ProductionJob>, getAllJobs, (), (const));
};
```

### Step 7-2: 재고 충분 → CONFIRMED 전환
**RED**
```cpp
TEST(OrderControllerTest, ApproveOrderChangesToConfirmedWhenStockSufficient) {
    // 주문량 100, 재고 200 → 재고 충분 → CONFIRMED
    MockSampleRepository mockSampleRepo;
    MockOrderRepository mockOrderRepo;
    MockProductionQueue mockQueue;
    OrderController controller(&mockSampleRepo, &mockOrderRepo, &mockQueue);
    Order order("ORD-001", "S-001", "고객", 100);
    Sample sample("S-001", "시료", 0.5, 0.92, 200);
    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockSampleRepo, findById("S-001")).WillOnce(Return(sample));
    EXPECT_CALL(mockOrderRepo, update(testing::_)).Times(1);
    EXPECT_CALL(mockSampleRepo, update(testing::_)).Times(1); // 재고 차감
    EXPECT_CALL(mockQueue, enqueue(testing::_)).Times(0);     // 생산 큐 등록 안 함
    auto result = controller.approveOrder("ORD-001");
    EXPECT_EQ(result, OrderStatus::CONFIRMED);
}
```
**GREEN**
**REVIEW**

### Step 7-3: 재고 부족 → PRODUCING + 생산 큐 등록
**RED**
```cpp
TEST(OrderControllerTest, ApproveOrderChangesToProducingWhenStockInsufficient) {
    // 주문량 200, 재고 30 → 부족분 170 → PRODUCING + 큐 등록
    MockSampleRepository mockSampleRepo;
    MockOrderRepository mockOrderRepo;
    MockProductionQueue mockQueue;
    OrderController controller(&mockSampleRepo, &mockOrderRepo, &mockQueue);
    Order order("ORD-001", "S-001", "고객", 200);
    Sample sample("S-001", "SiC 파워기판", 0.8, 0.92, 30);
    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockSampleRepo, findById("S-001")).WillOnce(Return(sample));
    EXPECT_CALL(mockOrderRepo, update(testing::_)).Times(1);
    EXPECT_CALL(mockQueue, enqueue(testing::_)).Times(1);
    auto result = controller.approveOrder("ORD-001");
    EXPECT_EQ(result, OrderStatus::PRODUCING);
}
```
**GREEN**
**REVIEW**

### Step 7-4: 주문 거절 → REJECTED
**RED**
```cpp
TEST(OrderControllerTest, RejectOrderChangesToRejected) {
    MockSampleRepository mockSampleRepo;
    MockOrderRepository mockOrderRepo;
    MockProductionQueue mockQueue;
    OrderController controller(&mockSampleRepo, &mockOrderRepo, &mockQueue);
    Order order("ORD-001", "S-001", "고객", 100);
    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockOrderRepo, update(testing::_)).Times(1);
    bool result = controller.rejectOrder("ORD-001");
    EXPECT_TRUE(result);
}
```
**GREEN**
**REVIEW**

### Step 7-5: RESERVED 주문 목록 조회
**RED**
```cpp
TEST(OrderControllerTest, GetPendingOrdersReturnsOnlyReserved) {
    MockSampleRepository mockSampleRepo;
    MockOrderRepository mockOrderRepo;
    MockProductionQueue mockQueue;
    OrderController controller(&mockSampleRepo, &mockOrderRepo, &mockQueue);
    std::vector<Order> allOrders = {
        Order("ORD-001", "S-001", "고객A", 100),  // RESERVED
        Order("ORD-002", "S-001", "고객B", 50),   // RESERVED
    };
    allOrders[1].changeStatus(OrderStatus::CONFIRMED);
    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(allOrders));
    auto pending = controller.getPendingOrders();
    EXPECT_EQ(pending.size(), 1);
    EXPECT_EQ(pending[0].orderId, "ORD-001");
}
```
**GREEN**
**REVIEW**

---

## Phase 8: ProductionController - 생산라인 관리

> 목표: FIFO 생산 큐 운영, 생산 현황 조회, 생산 완료 처리를 TDD로 구현

### Step 8-1: 생산 큐 FIFO 순서 보장
**RED**
```cpp
TEST(ProductionControllerTest, ProductionQueueFollowsFifoOrder) {
    ProductionQueue queue;
    ProductionJob job1("ORD-001", "S-001", 100, 0.92, 0.5);
    ProductionJob job2("ORD-002", "S-002", 50, 0.78, 0.3);
    queue.enqueue(job1);
    queue.enqueue(job2);
    EXPECT_EQ(queue.front()->orderId, "ORD-001");
    queue.dequeue();
    EXPECT_EQ(queue.front()->orderId, "ORD-002");
}
```
**GREEN** — `Model/ProductionQueue.h/.cpp` (std::queue 래퍼) 구현
**REVIEW**

### Step 8-2: 생산 현황 조회 (현재 처리 중)
**RED**
```cpp
TEST(ProductionControllerTest, GetCurrentJobReturnsFirstInQueue) {
    MockProductionQueue mockQueue;
    MockSampleRepository mockSampleRepo;
    MockOrderRepository mockOrderRepo;
    ProductionController controller(&mockQueue, &mockSampleRepo, &mockOrderRepo);
    ProductionJob job("ORD-001", "S-001", 170, 0.92, 0.8);
    EXPECT_CALL(mockQueue, front()).WillOnce(Return(job));
    auto current = controller.getCurrentJob();
    ASSERT_TRUE(current.has_value());
    EXPECT_EQ(current->orderId, "ORD-001");
    EXPECT_EQ(current->actualProduction, 206);
}
```
**GREEN** — `Controller/ProductionController.h/.cpp` 구현
**REVIEW**

### Step 8-3: 대기 큐 목록 조회
**RED**
```cpp
TEST(ProductionControllerTest, GetWaitingJobsReturnsQueueExcludingFirst) {
    MockProductionQueue mockQueue;
    MockSampleRepository mockSampleRepo;
    MockOrderRepository mockOrderRepo;
    ProductionController controller(&mockQueue, &mockSampleRepo, &mockOrderRepo);
    std::vector<ProductionJob> all = {
        ProductionJob("ORD-001", "S-001", 100, 0.92, 0.5),
        ProductionJob("ORD-002", "S-002", 50, 0.78, 0.3),
        ProductionJob("ORD-003", "S-003", 80, 0.88, 0.6)
    };
    EXPECT_CALL(mockQueue, getAllJobs()).WillOnce(Return(all));
    auto waiting = controller.getWaitingJobs();
    EXPECT_EQ(waiting.size(), 2); // 첫 번째(현재 처리 중)는 제외
}
```
**GREEN**
**REVIEW**

### Step 8-4: 생산 완료 처리 → CONFIRMED + 재고 증가
**RED**
```cpp
TEST(ProductionControllerTest, CompleteProductionChangesOrderToConfirmedAndIncreasesStock) {
    MockProductionQueue mockQueue;
    MockSampleRepository mockSampleRepo;
    MockOrderRepository mockOrderRepo;
    ProductionController controller(&mockQueue, &mockSampleRepo, &mockOrderRepo);
    ProductionJob job("ORD-001", "S-001", 170, 0.92, 0.8); // actualProduction=206
    Order order("ORD-001", "S-001", "고객", 200);
    order.changeStatus(OrderStatus::PRODUCING);
    Sample sample("S-001", "시료", 0.8, 0.92, 30);
    EXPECT_CALL(mockQueue, front()).WillOnce(Return(job));
    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockSampleRepo, findById("S-001")).WillOnce(Return(sample));
    EXPECT_CALL(mockSampleRepo, update(testing::_)).Times(1); // 재고 증가
    EXPECT_CALL(mockOrderRepo, update(testing::_)).Times(1); // CONFIRMED
    EXPECT_CALL(mockQueue, dequeue()).Times(1);
    bool result = controller.completeCurrentProduction();
    EXPECT_TRUE(result);
}
```
**GREEN**
**REVIEW**

---

## Phase 9: MonitoringController - 모니터링

> 목표: 상태별 주문 집계(REJECTED 제외), 재고 상태 판단을 TDD로 구현

### Step 9-1: 상태별 주문 건수 집계 (REJECTED 제외)
**RED**
```cpp
TEST(MonitoringControllerTest, CountOrdersByStatusExcludesRejected) {
    MockOrderRepository mockOrderRepo;
    MockSampleRepository mockSampleRepo;
    MonitoringController controller(&mockOrderRepo, &mockSampleRepo);
    std::vector<Order> orders = {
        makeOrder("ORD-001", OrderStatus::RESERVED),
        makeOrder("ORD-002", OrderStatus::CONFIRMED),
        makeOrder("ORD-003", OrderStatus::CONFIRMED),
        makeOrder("ORD-004", OrderStatus::REJECTED),   // 집계 제외
        makeOrder("ORD-005", OrderStatus::PRODUCING),
        makeOrder("ORD-006", OrderStatus::RELEASE),
    };
    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(orders));
    auto counts = controller.getOrderCounts();
    EXPECT_EQ(counts[OrderStatus::RESERVED], 1);
    EXPECT_EQ(counts[OrderStatus::CONFIRMED], 2);
    EXPECT_EQ(counts[OrderStatus::PRODUCING], 1);
    EXPECT_EQ(counts[OrderStatus::RELEASE], 1);
    EXPECT_EQ(counts.count(OrderStatus::REJECTED), 0); // REJECTED 없음
}
```
**GREEN** — `Controller/MonitoringController.h/.cpp` 구현
**REVIEW**

### Step 9-2: 재고 상태 - 고갈 (stock == 0)
**RED**
```cpp
TEST(MonitoringControllerTest, StockStatusIsDepletedWhenZero) {
    MockOrderRepository mockOrderRepo;
    MockSampleRepository mockSampleRepo;
    MonitoringController controller(&mockOrderRepo, &mockSampleRepo);
    Sample s("S-005", "산화막 웨이퍼", 0.6, 0.88, 0);
    EXPECT_CALL(mockSampleRepo, findAll()).WillOnce(Return(std::vector<Sample>{s}));
    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(std::vector<Order>{}));
    auto statuses = controller.getStockStatuses();
    EXPECT_EQ(statuses["S-005"], StockStatus::DEPLETED);
}
```
**GREEN** — `StockStatus` enum (SUFFICIENT / SHORTAGE / DEPLETED) 추가
**REVIEW**

### Step 9-3: 재고 상태 - 부족 (재고 < 주문 잔량)
**RED**
```cpp
TEST(MonitoringControllerTest, StockStatusIsShortageWhenLessThanPendingOrders) {
    // 재고 30, CONFIRMED 주문 200 → 부족
    MockOrderRepository mockOrderRepo;
    MockSampleRepository mockSampleRepo;
    MonitoringController controller(&mockOrderRepo, &mockSampleRepo);
    Sample s("S-003", "SiC 파워기판", 0.8, 0.92, 30);
    Order confirmedOrder("ORD-001", "S-003", "고객", 200);
    confirmedOrder.changeStatus(OrderStatus::CONFIRMED);
    EXPECT_CALL(mockSampleRepo, findAll()).WillOnce(Return(std::vector<Sample>{s}));
    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(std::vector<Order>{confirmedOrder}));
    auto statuses = controller.getStockStatuses();
    EXPECT_EQ(statuses["S-003"], StockStatus::SHORTAGE);
}
```
**GREEN**
**REVIEW**

### Step 9-4: 재고 상태 - 여유 (재고 ≥ 주문 잔량)
**RED**
```cpp
TEST(MonitoringControllerTest, StockStatusIsSufficientWhenStockCoversOrders) {
    // 재고 480, CONFIRMED 주문 100 → 여유
    ...
    EXPECT_EQ(statuses["S-001"], StockStatus::SUFFICIENT);
}
```
**GREEN**
**REVIEW**

---

## Phase 10: ReleaseController - 출고 처리

> 목표: CONFIRMED 주문 조회 및 RELEASE 전환을 TDD로 구현

### Step 10-1: CONFIRMED 주문 목록 조회
**RED**
```cpp
TEST(ReleaseControllerTest, GetReleasableOrdersReturnsOnlyConfirmed) {
    MockOrderRepository mockOrderRepo;
    MockSampleRepository mockSampleRepo;
    ReleaseController controller(&mockOrderRepo, &mockSampleRepo);
    std::vector<Order> all = {
        makeOrder("ORD-001", OrderStatus::CONFIRMED),
        makeOrder("ORD-002", OrderStatus::PRODUCING),
        makeOrder("ORD-003", OrderStatus::CONFIRMED),
    };
    EXPECT_CALL(mockOrderRepo, findAll()).WillOnce(Return(all));
    auto releasable = controller.getReleasableOrders();
    EXPECT_EQ(releasable.size(), 2);
}
```
**GREEN** — `Controller/ReleaseController.h/.cpp` 구현
**REVIEW**

### Step 10-2: 출고 처리 → RELEASE 전환
**RED**
```cpp
TEST(ReleaseControllerTest, ReleaseOrderChangesStatusToRelease) {
    MockOrderRepository mockOrderRepo;
    MockSampleRepository mockSampleRepo;
    ReleaseController controller(&mockOrderRepo, &mockSampleRepo);
    Order order("ORD-001", "S-001", "고객", 150);
    order.changeStatus(OrderStatus::CONFIRMED);
    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockOrderRepo, update(testing::_)).Times(1);
    bool result = controller.releaseOrder("ORD-001");
    EXPECT_TRUE(result);
}
```
**GREEN**
**REVIEW**

### Step 10-3: CONFIRMED 아닌 주문 출고 거부
**RED**
```cpp
TEST(ReleaseControllerTest, CannotReleaseOrderThatIsNotConfirmed) {
    MockOrderRepository mockOrderRepo;
    MockSampleRepository mockSampleRepo;
    ReleaseController controller(&mockOrderRepo, &mockSampleRepo);
    Order order("ORD-001", "S-001", "고객", 150); // RESERVED 상태
    EXPECT_CALL(mockOrderRepo, findById("ORD-001")).WillOnce(Return(order));
    EXPECT_CALL(mockOrderRepo, update(testing::_)).Times(0);
    bool result = controller.releaseOrder("ORD-001");
    EXPECT_FALSE(result);
}
```
**GREEN**
**REVIEW**

---

## Phase 11: View 레이어 - 콘솔 출력

> 목표: Controller 결과를 받아 콘솔에 출력하는 View 구현 (TDD 대상 아님, 수동 확인)

View 레이어는 순수 출력 담당이므로 TDD 적용 범위 밖.
각 View 구현 후 수동 실행으로 화면 레이아웃 확인.

| View 파일 | 구현 내용 |
|-----------|-----------|
| `MainView.h/.cpp` | 시스템 현황 요약 + 메뉴 출력 |
| `SampleView.h/.cpp` | 시료 목록 테이블 (페이징 5개씩) |
| `OrderView.h/.cpp` | 주문 입력 폼 + 접수 결과 출력 |
| `ApprovalView.h/.cpp` | RESERVED 목록 + 재고 확인 결과 출력 |
| `MonitoringView.h/.cpp` | 상태별 건수 + 재고 상태 바(bar) 출력 |
| `ProductionView.h/.cpp` | 현재 처리 중 + 대기 큐 테이블 출력 |
| `ReleaseView.h/.cpp` | CONFIRMED 목록 + 출고 결과 출력 |

---

## Phase 12: 통합 시나리오 테스트

> 목표: 실제 Repository(파일)를 사용한 End-to-End 시나리오 검증

### 시나리오 1: 재고 충분 → 즉시 출고 흐름
```
시료 등록(S-001, 재고 500)
→ 주문 접수(200개) → RESERVED
→ 주문 승인 → 재고 500≥200 → CONFIRMED + 재고 300
→ 출고 처리 → RELEASE
```

### 시나리오 2: 재고 부족 → 생산 → 출고 흐름
```
시료 등록(S-003, 재고 30)
→ 주문 접수(200개) → RESERVED
→ 주문 승인 → 재고 30<200 → PRODUCING + 생산 큐 등록(실생산량 206)
→ 생산 완료 처리 → CONFIRMED + 재고 증가
→ 출고 처리 → RELEASE
```

### 시나리오 3: 주문 거절 흐름
```
주문 접수 → RESERVED
→ 주문 거절 → REJECTED
→ 모니터링 집계에서 REJECTED 제외 확인
```

### 시나리오 4: 데이터 영속성 확인
```
시료 등록 + 주문 접수 후 프로세스 종료
→ 재시작 후 동일 데이터 로드 확인
```

### 시나리오 5: 생산 큐 FIFO 확인
```
주문 3건 연속 PRODUCING 등록
→ 생산라인 조회에서 등록 순서대로 표시 확인
```

---

## Phase 13: DummyDataGenerator 도구

> 목표: 테스트용 초기 데이터를 JSON 파일로 생성하는 도구 구현

### Step 13-1: 시료 더미 데이터 생성
**RED**
```cpp
TEST(DummyDataGeneratorTest, GeneratesSampleDataFile) {
    DummyDataGenerator gen("test_data/");
    gen.generateSamples(5);
    SampleRepository repo("test_data/samples.json");
    auto samples = repo.findAll();
    EXPECT_EQ(samples.size(), 5);
}
```
**GREEN** — `Tools/DummyDataGenerator.h/.cpp` 구현
**REVIEW**

### Step 13-2: 주문 더미 데이터 생성 (다양한 상태)
**RED**
```cpp
TEST(DummyDataGeneratorTest, GeneratesOrdersWithVariousStatuses) {
    DummyDataGenerator gen("test_data/");
    gen.generateOrders(10);
    OrderRepository repo("test_data/orders.json");
    auto orders = repo.findAll();
    EXPECT_EQ(orders.size(), 10);
    // 다양한 상태 포함 확인
    bool hasReserved = false, hasConfirmed = false;
    for (auto& o : orders) {
        if (o.status == OrderStatus::RESERVED) hasReserved = true;
        if (o.status == OrderStatus::CONFIRMED) hasConfirmed = true;
    }
    EXPECT_TRUE(hasReserved);
    EXPECT_TRUE(hasConfirmed);
}
```
**GREEN**
**REVIEW**

---

## Phase 14: DataMonitor 도구

> 목표: 저장된 JSON 데이터를 콘솔에서 실시간 조회하는 관리자 도구 구현

- `Tools/DataMonitor.h/.cpp`
- 시료 목록 전체 출력
- 주문 목록 상태별 필터 출력
- View 레이어와 동일하게 수동 확인으로 검증

---

## Phase 15: main.cpp - 진입점 및 메인 루프

> 목표: 모든 Controller·View를 조합하여 콘솔 앱 완성

```cpp
// main.cpp 구조
int main() {
    // Repository 초기화 (JSON 파일 경로 설정)
    SampleRepository sampleRepo("data/samples.json");
    OrderRepository orderRepo("data/orders.json");
    ProductionQueue productionQueue;

    // Controller 초기화 (의존성 주입)
    SampleController sampleCtrl(&sampleRepo);
    OrderController orderCtrl(&sampleRepo, &orderRepo, &productionQueue);
    ProductionController prodCtrl(&productionQueue, &sampleRepo, &orderRepo);
    MonitoringController monCtrl(&orderRepo, &sampleRepo);
    ReleaseController releaseCtrl(&orderRepo, &sampleRepo);

    // View 초기화
    MainView mainView;

    // 메인 루프
    while (true) {
        mainView.display(sampleRepo, orderRepo, productionQueue);
        int choice = mainView.getMenuChoice();
        if (choice == 0) break;
        // 메뉴 라우팅 ...
    }
}
```

---

## 전체 개발 순서 요약

```
Phase 0  : 테스트 환경 구축
Phase 1  : Sample 모델
Phase 2  : Order 모델
Phase 3  : ProductionJob 모델 + 생산량 공식
Phase 4  : Repository 레이어 (JSON 영속성)
Phase 5  : SampleController (gmock)
Phase 6  : OrderController - 주문 접수 (gmock)
Phase 7  : OrderController - 승인/거절 (gmock)
Phase 8  : ProductionController (gmock)
Phase 9  : MonitoringController (gmock)
Phase 10 : ReleaseController (gmock)
Phase 11 : View 레이어 (수동 확인)
Phase 12 : 통합 시나리오 테스트
Phase 13 : DummyDataGenerator 도구
Phase 14 : DataMonitor 도구
Phase 15 : main.cpp 진입점 완성
```

## 각 Phase 완료 체크리스트 (SKILL.md 기반)

- [ ] 모든 새 메서드에 테스트 존재
- [ ] 각 테스트가 실패하는 것을 먼저 확인 (RED 단계)
- [ ] 올바른 이유로 실패하는지 확인 (기능 미구현, 오타 아님)
- [ ] 테스트 통과를 위한 최소한의 코드만 작성 (GREEN 단계)
- [ ] 모든 테스트 통과 확인
- [ ] 빌드 경고 없음 확인
- [ ] REVIEW 후에도 GREEN 유지
- [ ] Mock은 인터페이스 기반으로만 사용
