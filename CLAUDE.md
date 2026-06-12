# 반도체 시료 생산주문관리 시스템 (SampleOrderSystem)

## 프로젝트 개요

가상의 반도체 회사 **S-Semi**의 시료(Sample) 생산주문을 체계적으로 관리하기 위한 콘솔 기반 C++ 애플리케이션.
주문 급증으로 인한 혼선을 방지하고, 주문·재고·생산라인 현황을 한눈에 파악하는 것이 목표.

## 배경 (page 5)

S-Semi는 다양한 종류의 반도체 시료(Sample)를 생산하여 연구소, 팹리스(Fabless) 업체, 대학 연구실 등의 고객에게 납품한다.
시료는 주문이 들어오면 웨이퍼 공정 설비를 통해 제작되고, 검수를 거쳐 고객에게 출고된다.

주문량 급증으로 아래와 같은 문제가 발생했다:
- "어, 이 주문 처리됐나요?"
- "공정 예약을 했는데, 언제 완성되는지 모르겠어요."
- "이미 충분한 시료 재고가 있는데, 왜 추가 공정이 돌아가고 있나요?"

엑셀과 메모장으로 주문을 관리하다 보니 실수가 잦고, 재고와 공정 현황을 한눈에 파악하기 어려웠다.
이를 해결하기 위해 **반도체 시료 생산주문관리 시스템** 개발을 결정했다.

## 역할별 흐름도 (page 6)

```
고객 (시료 요청자)
    │  시료 요청
    ▼
주문 담당자 (주문서 관리)
    │  주문서 전달
    ▼
생산 담당자 (시료 생산·승인)
    │  승인 / 거절
    └──────────────→ 주문 담당자
```

| 역할 | 담당 업무 |
|------|-----------|
| 고객 | 필요한 시료를 이메일로 요청 |
| 주문 담당자 | 요청에 맞게 주문서 작성 및 관리 |
| 생산 담당자 | 개발 시료 등록, 주문 수신 후 승인 또는 거절 |

## 시스템 개요 (page 7)

### 생산 라인
- 공장에서 시료 하나를 생산하는 설비 흐름
- **하나의 생산 라인은 시료를 하나씩 생산** (단일 라인)
- 주문이 들어온 시료에 대해서만 생산

### 운영 방식
- 시스템은 **콘솔 기반**으로 동작
- 담당자가 직접 명령을 입력하여 시료를 등록하고 주문을 처리

## 기술 스택 및 빌드 환경

| 항목 | 내용 |
|------|------|
| 언어 | C++20 |
| IDE | Visual Studio 2022 |
| 빌드 도구 | MSBuild (v145 toolset) |
| 플랫폼 | Windows 10+, x64 / Win32 |
| 앱 타입 | Console Application |
| 솔루션 파일 | `SampleOrderSystem-HJOY.slnx` |
| 프로젝트 파일 | `SampleOrderSystem-HJOY/SampleOrderSystem-HJOY.vcxproj` |

### 빌드 명령
```powershell
# Debug x64 빌드
msbuild SampleOrderSystem-HJOY.slnx /p:Configuration=Debug /p:Platform=x64

# Release x64 빌드
msbuild SampleOrderSystem-HJOY.slnx /p:Configuration=Release /p:Platform=x64
```

## 아키텍처: MVC 패턴

PoC(ConsoleMVC-HJOY)에서 검증한 MVC 패턴을 그대로 적용한다.

```
SampleOrderSystem-HJOY/
├── Model/               # 데이터 구조 및 비즈니스 로직
│   ├── Sample.h/.cpp        # 시료 엔티티
│   ├── Order.h/.cpp         # 주문 엔티티
│   ├── ProductionLine.h/.cpp # 생산라인 엔티티
│   └── Inventory.h/.cpp     # 재고 관리
├── View/                # 콘솔 출력 (입출력 담당)
│   ├── MainView.h/.cpp
│   ├── SampleView.h/.cpp
│   ├── OrderView.h/.cpp
│   ├── MonitoringView.h/.cpp
│   ├── ProductionView.h/.cpp
│   └── ReleaseView.h/.cpp
├── Controller/          # 사용자 입력 처리 및 흐름 제어
│   ├── MainController.h/.cpp
│   ├── SampleController.h/.cpp
│   ├── OrderController.h/.cpp
│   ├── MonitoringController.h/.cpp
│   ├── ProductionController.h/.cpp
│   └── ReleaseController.h/.cpp
├── Repository/          # 데이터 영속성 (DataPersistence-HJOY PoC 기반)
│   ├── IRepository.h        # CRUD 인터페이스
│   ├── SampleRepository.h/.cpp
│   ├── OrderRepository.h/.cpp
│   └── JsonSerializer.h/.cpp # JSON 직렬화
├── Tools/               # 유틸리티 도구
│   ├── DataMonitor.h/.cpp   # 데이터 모니터링 (DataMonitor-HJOY PoC 기반)
│   └── DummyDataGenerator.h/.cpp # 더미 데이터 생성 (DummyDataGenerator-HJOY PoC 기반)
├── data/                # 영속성 데이터 저장 디렉토리
│   ├── samples.json
│   └── orders.json
└── main.cpp
```

### MVC 역할 분리 원칙
- **Model**: 순수 데이터와 비즈니스 규칙만 담당. View/Controller에 의존하지 않음
- **View**: 콘솔 출력과 사용자 입력 수집만 담당. 로직 없음
- **Controller**: Model과 View를 연결. 흐름 제어 및 유효성 검사 담당
- **Repository**: 파일 I/O를 추상화. Controller는 Repository 인터페이스만 사용

## 도메인 모델

### 시료 (Sample)
```cpp
struct Sample {
    std::string id;              // 시료 ID (예: S-001)
    std::string name;            // 시료명 (예: 실리콘 웨이퍼-8인치)
    double avgProductionTime;    // 평균 생산시간 (분/ea)
    double yieldRate;            // 수율 (0.0 ~ 1.0, 예: 0.92)
    int stock;                   // 현재 재고 수량 (ea)
};
```

### 주문 (Order)
```cpp
enum class OrderStatus {
    RESERVED,    // 주문 접수
    REJECTED,    // 주문 거절
    PRODUCING,   // 생산 중 (재고 부족으로 생산라인 등록)
    CONFIRMED,   // 출고 대기 (재고 충분 or 생산 완료)
    RELEASE      // 출고 완료
};

struct Order {
    std::string orderId;      // 주문번호 (예: ORD-20260416-0001)
    std::string sampleId;     // 시료 ID
    std::string customerName; // 고객명
    int quantity;             // 주문 수량
    OrderStatus status;       // 주문 상태
    std::time_t createdAt;    // 주문 접수 시각
    std::time_t updatedAt;    // 상태 변경 시각
};
```

### 생산라인 (ProductionLine)
```cpp
struct ProductionJob {
    std::string orderId;      // 연결된 주문 ID
    std::string sampleId;     // 생산할 시료 ID
    int shortage;             // 부족분 (주문량 - 현재 재고)
    int actualProduction;     // 실 생산량 = ceil(shortage / (yieldRate * 0.9))
    double totalProductionTime; // 총 생산 시간 = avgProductionTime * actualProduction
};

// 생산 큐: FIFO 방식
// std::queue<ProductionJob> productionQueue;
```

## 주문 상태 흐름

```
[고객 주문 접수]
      │
      ▼
  RESERVED ──── [거절] ──→ REJECTED
      │
      │ [승인]
      ▼
  재고 확인
  ├── 재고 충분 ──→ CONFIRMED ──→ [출고 처리] ──→ RELEASE
  └── 재고 부족 ──→ PRODUCING ──→ [생산 완료] ──→ CONFIRMED ──→ [출고 처리] ──→ RELEASE
```

| 상태 | 의미 |
|------|------|
| RESERVED | 주문 접수 (초기 상태) |
| REJECTED | 주문 거절 (정상 흐름 외, 모니터링 제외) |
| PRODUCING | 승인 완료 + 재고 부족 → 생산 중 |
| CONFIRMED | 승인 완료 + 출고 대기 |
| RELEASE | 출고 완료 |

## 재고 계산 부연 설명
- 재고에는 생산이 완료된 개수도 반영되어야 한다
- 예를 들어 재고가 0인 상태에서 100개를 주문 접수하고 실 생산량이 110 만큼 생산하는중에 70개 생산이 완료된 시점에서 5개 주문 요청이 들어오면 실 생산양 6개를 주문 접수한다
- 예를 들어 재고가 0인 상태에서 100개를 주문 접수하고 실 생산량이 110 만큼 생산하는중에 106개 생산이 완료된 시점에서 5개 주문 요청이 들어오면 승인 완료한다

## 생산량 계산 공식

```
실 생산량 = ceil(부족분 / (수율 * 0.9))
총 생산시간 = 평균 생산시간(분/ea) * 실 생산량
```

- **부족분** = 주문 수량 - 현재 재고
- **수율** = 시료별 yieldRate (예: 0.92)
- 0.9는 추가 오차(불량) 보정 계수

## 기능 명세

## 메인 메뉴 구성

메인 메뉴는 두 가지 역할을 동시에 수행한다:
1. **기능 선택 화면 표시** — 번호 입력으로 각 기능으로 이동
2. **전체 시료 요약 정보 표시** — 등록 시료 수, 총 재고, 전체 주문 건수, 생산라인 대기 수를 상단에 항상 표시

### 메뉴 항목 정의

| 메뉴 | 의미 |
|------|------|
| 시료 관리 | 새로운 시료 등록, 목록 조회, 이름 검색 기능 |
| 시료 주문 | 고객 주문 접수 — 시료 ID, 고객명, 주문 수량 입력 |
| 주문 접수/승인/거절 | 고객 주문 접수 및 생산 라인 담당자의 승인·거절 처리 |
| 모니터링 | 상태별 주문 수 및 시료별 재고 현황 확인 |
| 출고 처리 | CONFIRMED 상태 주문에 대해 출고 실행 |
| 생산 라인 | 현재 생산 중인 시료 및 대기 중인 생산 큐 확인 |

### 예시 UI 레이아웃

```
반도체 시료 생산주문관리 시스템
================================================
시스템 현황  YYYY-MM-DD HH:MM:SS

등록 시료  N종       총 재고  N ea
전체 주문  N건       생산라인 N건 대기

[1] 시료 관리          [2] 시료 주문
[3] 주문 승인/거절     [4] 모니터링
[5] 생산라인 조회      [6] 출고 처리
[0] 종료

선택 > _
```

### [1] 시료 관리 (SampleController)
메뉴는 두 가지 역할을 동시에 수행한다:
1. 시료(Sample)는 이 시스템의 가장 기본이 되는단위
2. 각 시료는 고유한 이름과 속성을 가지며, 시스템에 등록된 시료만 주문 가능

| 서브메뉴 | 기능 |
|----------|------|
| 시료 등록 | 시료 ID, 이름, 평균 생산시간, 수율 입력 후 저장 |
| 시료 조회 | 등록된 전체 시료 목록 + 현재 재고 수량 표시 (페이징) |
| 시료 검색 | 이름 등 속성 기반 검색 |

#### 예시 UI 화면 (시료 목록 조회)

```
================================================================
[1] 시료 관리

[1] 시료 등록   [2] 시료 목록   [3] 시료 검색   [0] 위로
선택 > 2

등록 시료 목록  (총 12종)

ID        시료명                평균 생산시간   수율    현재 재고
S-001     실리콘 웨이퍼-8인치   0.5 min/ea     0.92    480 ea
S-002     GaN 에피택셀-4인치    0.3 min/ea     0.78    220 ea
S-003     SiC 파워기판-6인치    0.8 min/ea     0.92     30 ea
S-004     포토레지스트-PR7      0.2 min/ea     0.95    910 ea
S-005     산화막 웨이퍼-SiO2   0.6 min/ea     0.88      0 ea
...외 7종   [N] 다음페이지
선택 > _
```

**시료 목록 표시 규칙:**
- 페이지당 5개씩 표시, `[N]` 입력으로 다음 페이지 이동
- 표시 컬럼: ID, 시료명, 평균 생산시간(min/ea), 수율, 현재 재고(ea)

### [2] 시료 주문 (OrderController - 접수)
메뉴는 역할을 동시에 수행한다:
1. 고객이 시료를 요청하면 주문 담당자가 주문을 생성

- 시료예약
  고객이 원하는 시료와 수량을 주문
  이 시점에서 주문 상태는 RESERVED

- 예약시 입력값
  시료ID
  고객명
  주문수량

- 시료 ID, 고객명, 주문 수량 입력
- 입력 내용 확인 후 접수 → 주문번호 발급, 상태 RESERVED
- 주문번호 형식: `ORD-YYYYMMDD-NNNN`

#### 예시 UI 화면

```
================================================================
[2] 시료 주문

시료 ID    > S-003
고객명     > 삼성전자 파운드리
주문 수량  > 200

입력 내용 확인
시료       SiC 파워기판-6인치  (S-003)
고객       삼성전자 파운드리
수량       200 ea

[Y] 예약 접수   [N] 취소
선택 > Y

예약 접수 완료.

주문번호   ORD-20260416-0043
현재 상태  RESERVED

※ 재고 확인은 [3] 승인 메뉴에서 직접 진행하세요.

선택 > _
```

**주문 접수 규칙:**
- 시료 ID는 시스템에 등록된 ID만 허용 (미등록 ID 입력 시 오류 안내)
- 입력 내용 확인 단계에서 `[N]` 선택 시 접수 취소 (데이터 저장 안 됨)
- 접수 완료 후 재고 확인 및 승인은 `[3] 주문 승인/거절` 메뉴에서 처리

### [3] 주문 승인/거절 (OrderController - 승인)
메뉴는 두 가지 역할을 동시에 수행한다:
1. 접수된 주문(RESERVED) 목록을 확인
2. 특정 주문에 대하여 승인 혹은 거절

- 접수된 주문 목록
  RESERVED 상태의 주문 목록 Display

- 주문 승인
  접수된 특정 주문에 대해 승인
    승인시 재고 상황에 따라 2가지 방식으로 자동으로 처리
    재고가 충분한 경우→ 주문을 즉시 CONFIRMED 상태로 전환
    재고가 부족한 경우→ 생산 라인에 자동으로 등록, 주문 상태를 PRODUCING으로 전환

- 주문 거절
  접수된 특정 주문에 대해 거절
  즉시 REJECTED 상태로 전환


- RESERVED 상태 주문 목록 표시
- 선택한 주문 승인 → 재고 자동 판단:
  - 재고 ≥ 주문량: 즉시 CONFIRMED (재고에서 차감)
  - 재고 < 주문량: 생산 큐에 등록, PRODUCING으로 전환
- 선택한 주문 거절 → 즉시 REJECTED

#### 예시 UI 화면

```
================================================================
[3] 주문 승인/거절

승인 대기 중인 예약 목록  (RESERVED)

변호   주문번호    고객               시료                  수량      상태
[1]   ORD-0041   LG이노텍           산화막 웨이퍼-SiO2    300 ea   RESERVED
[2]   ORD-0042   SK하이닉스         실리콘 웨이퍼-8인치   150 ea   RESERVED
[3]   ORD-0043   삼성전자 파운드리  SiC 파워기판-6인치    200 ea   RESERVED

승인할 번호 > 3

재고 확인 중...

시료       SiC 파워기판-6인치   현재 재고  30 ea
주문 수량  200 ea               부족분     170 ea  ← 이 수량만큼 생산

재고 부족.  부족분 170 ea 승인하시겠습니까?  (실생산량 206 ea / 165 min)

[Y] 승인   [N] 주문 거절
선택 > Y

승인 완료.

상태 변경   RESERVED → PRODUCING
주문번호    ORD-20260416-0043
```

**승인 처리 규칙:**
- 재고 확인 후 부족분과 실생산량·총 생산시간을 화면에 표시한 뒤 최종 승인 여부를 확인
- 재고 부족 시 `[Y]` 승인 → 생산 큐 등록 + 상태 PRODUCING 전환
- 재고 부족 시 `[N]` 선택 → 주문 거절(REJECTED) 처리
- 재고 충분 시 확인 없이 즉시 CONFIRMED 전환 후 재고 차감

### [4] 모니터링 (MonitoringController)
메뉴는 두 가지 역할을 동시에 수행한다:
1. 주문에 따른 시료 재고에 대한 정보 모니터링 기능
2. 담당자가 현재 시스템의 상태를 한눈에 파악할 수 있도록 구성

- 주문량확인
  현재상태별(RESERVED/CONFIRMED/ PRODUCING / RELEASE) 목록을 확인
  REJECTED 는 유효한 주문이 아니므로 무시

- 재고량 확인
  각 시료별 현재 재고 수량을 확인
  주문 대비 재고 수량에 따라 상태도 표기
    여유: 주문대비 재고충분상태
    부족: 주문대비 재고수량부족상태
    고갈: 수량이 0인 상태


| 서브기능 | 내용 |
|----------|------|
| 주문량 확인 | 상태별(RESERVED/CONFIRMED/PRODUCING/RELEASE) 주문 건수 표시 |
| 재고량 확인 | 시료별 재고 수량 + 재고 상태 표기 |

**재고 상태 기준:**
- **여유**: 주문대비 재고 충분
- **부족**: 주문대비 재고 수량 부족
- **고갈**: 수량 0

#### 예시 UI 화면

```
================================================================
[4] 모니터링   2026-04-16 09:32:15

[1] 주문량 확인   [2] 재고량 확인   [0] 위로
선택 > 1

상태별 주문 현황
RESERVED     3건
CONFIRMED    8건
PRODUCING    3건  ← 생산라인 대기
RELEASE     18건

재고 현황

시료명                재고      상태    잔여율
실리콘 웨이퍼-8인치   480 ea    여유    ████████░░  80%
GaN 에피택셀-4인치    220 ea    여유    ██████░░░░  44%
SiC 파워기판-6인치     30 ea    부족    █░░░░░░░░░   6%
산화막 웨이퍼-SiO2      0 ea    고갈    ░░░░░░░░░░   0%

선택 > _
```

**모니터링 표시 규칙:**
- 주문량 확인과 재고량 확인은 서브메뉴로 분리하여 선택 조회
- REJECTED 주문은 상태별 집계에서 제외
- PRODUCING 건수 옆에 `← 생산라인 대기` 안내 문구 표시
- 재고 잔여율은 텍스트 바(bar) 형태로 시각화
- 재고 상태 색상 구분: 여유(정상), 부족(경고), 고갈(위험)

### [5] 생산라인 조회 (ProductionController)
메뉴는 두 가지 역할을 동시에 수행한다:
1. 생산라인에 대한 정보를 Display
2. 주문량에 대한 부족분을 생산하되, 수율 및 오차를 고려하여 시료를 생산
  실 생산량: ceil(부족분 / (수율 * 0.9))
  총생산 시간: 평균생산시간* 실생산량
  생산 완료시 주문 상태 PRODUCING -> CONFIRMED 변경

- 생산현황표기
  현재 생산중인 시료에 대한 정보 표기
  표기할 정보 수준은 자율적으로 결정
    ex) 주문 정보, 현재까지의 생산량 등

- 대기주문확인
  생산라인의 대기열인 생산큐를 이용
  생산작업을 대기하고 있는 목록을 출력
    스케쥴링 전략: FIFO

- 현재 생산 중인 시료 정보 표시 (주문 정보, 부족분, 실 생산량, 총 생산시간)
- 대기 중인 생산 큐 목록 표시 (FIFO 순서)
- 생산 진행 상황 표기 (진행률)
- 생산 완료 처리: PRODUCING → CONFIRMED, 재고 증가

#### 예시 UI 화면

```
================================================================
[5] 생산라인 조회   FIFO 방식

생산라인 1개 (단일 라인)   현재 상태: RUNNING

현재 처리 중

  주문번호  ORD-20260416-0038   시료  SiC 파워기판-6인치
  주문량    80 ea    재고 30 ea  →  부족 50 ea  →  실생산량 61 ea  (수율 0.92 / 49 min)
  진행      ████████░░  72%   완료 예정 09:49

대기 중인 주문  (FIFO 순)

순서   주문번호    시료                  주문량    부족분    실생산량   예상 완료
1      ORD-0040   산화막 웨이퍼-SiO2    150 ea    150 ea    190 ea     11:43
2      ORD-0043   SiC 파워기판-6인치    200 ea    170 ea    206 ea     14:28
3      ORD-0044   GaN 에피택셀-4인치    300 ea     80 ea    114 ea     15:02

* 부족분 = 주문량 - 재고,   실생산량 = ceil(부족분 / (수율 * 0.9))
* 선입선출(FIFO) 방식으로 처리됩니다.
선택 > _
```

**생산라인 표시 규칙:**
- 생산라인은 단일 라인(1개)으로 시료를 하나씩 순차 생산
- 현재 처리 중인 항목: 주문번호·시료·주문량·재고·부족분·실생산량·수율·총 생산시간·진행률·완료 예정 시각 표시
- 진행률은 텍스트 바(bar) 형태로 시각화
- 대기 큐: 순서·주문번호·시료·주문량·부족분·실생산량·예상 완료 시각 표시
- 생산 완료 시 주문 상태 PRODUCING → CONFIRMED 자동 전환 + 재고 증가

### [6] 출고 처리 (ReleaseController)
메뉴는 역할을 동시에 수행한다:
1. 재고가 충분해진 CONFIRMED 주문에 대하여 출고를 처리
  특정 주문에 대해 출고를 실행
  주문 상태가 RELEASE로 전환

- CONFIRMED 상태 주문 목록 표시
- 선택한 주문 출고 처리 → RELEASE로 전환 + 재고 차감
- 처리 결과 표시 (주문번호, 출고 수량, 처리 일시)

#### 예시 UI 화면

```
================================================================
[6] 출고 처리

출고 가능 주문  (CONFIRMED)

번호   주문번호       고객        시료                  수량
[1]   ORD-0042      SK하이닉스  실리콘 웨이퍼-8인치   150 ea
[2]   ORD-0035      DB하이텍    포토레지스트-PR7       400 ea

출고할 번호 > 1

출고 처리 완료.

주문번호   ORD-20260416-0042
출고수량   150 ea
처리일시   2026-04-16 09:34:02
상태       CONFIRMED → RELEASE

선택 > _
```

**출고 처리 규칙:**
- CONFIRMED 상태 주문만 출고 가능 목록에 표시
- 출고 완료 후 주문 상태 CONFIRMED → RELEASE 전환
- 처리 결과로 주문번호·출고수량·처리일시·상태 변경 내역 출력

## 데이터 영속성 (DataPersistence-HJOY PoC 기반)

### 저장 방식: JSON 파일
- 프로그램 종료 후 재시작 시에도 데이터 유지
- 저장 경로: `data/samples.json`, `data/orders.json`

### IRepository 인터페이스
```cpp
template<typename T>
class IRepository {
public:
    virtual void save(const T& entity) = 0;
    virtual T findById(const std::string& id) = 0;
    virtual std::vector<T> findAll() = 0;
    virtual void update(const T& entity) = 0;
    virtual void remove(const std::string& id) = 0;
    virtual void saveAll(const std::vector<T>& entities) = 0;
    virtual ~IRepository() = default;
};
```

### JSON 직렬화
- 외부 라이브러리 없이 표준 C++로 구현 (또는 nlohmann/json 헤더온리 라이브러리 사용)
- 파일 read/write 시 예외 처리 필수

## 데이터 모니터링 도구 (DataMonitor-HJOY PoC 기반)

실시간 데이터 상태를 콘솔에서 조회하는 관리자 도구.
메인 메뉴에 통합되거나 별도 실행 가능.

- 현재 저장된 samples.json, orders.json 내용 출력
- 필터 조회 (상태별, 시료별)
- 콘솔 테이블 포맷으로 가독성 있게 출력

## 더미 데이터 생성 도구 (DummyDataGenerator-HJOY PoC 기반)

테스트용 더미 데이터를 생성하여 JSON 파일에 저장.

**생성 대상:**
- 시료 데이터: 실제 반도체 시료명 기반 (실리콘 웨이퍼, GaN 에피택셜, SiC 파워기판 등)
- 주문 데이터: 다양한 상태(RESERVED/CONFIRMED/PRODUCING/RELEASE)의 주문

## 코드 작성 규칙

### 파일 구성
- 헤더(.h): 클래스 선언만
- 구현(.cpp): 메서드 구현
- 인라인 함수는 .h에 허용

### 네이밍 컨벤션
- 클래스: `PascalCase` (예: `SampleController`)
- 메서드/변수: `camelCase` (예: `getOrderById`)
- 상수: `UPPER_SNAKE_CASE` (예: `MAX_STOCK`)
- 멤버 변수: `m_` 접두사 (예: `m_sampleId`)

### 핵심 원칙
- 주석은 WHY가 명확할 때만 작성 (WHAT은 코드로 표현)
- 함수 하나에 하나의 책임
- View는 출력만, Controller는 흐름만, Model은 데이터만
- 재고 차감/증가는 반드시 Repository를 통해 처리 (직접 수정 금지)

## PoC 연계 구현 가이드

### ConsoleMVC-HJOY → 아키텍처 기반
- Model/View/Controller 디렉토리 구조 그대로 채택
- Controller가 View에서 입력받아 Model 조작 후 View로 출력하는 흐름

### DataPersistence-HJOY → Repository 레이어
- `IRepository<T>` 인터페이스 기반 CRUD 구현
- JSON 파일 저장/불러오기 로직 재활용
- 앱 시작 시 JSON 로드, 상태 변경 시마다 즉시 JSON 저장

### DataMonitor-HJOY → 모니터링 기능
- 상태별 주문 집계 쿼리 패턴 재활용
- 재고 상태(여유/부족/고갈) 판단 로직 재활용
- 콘솔 테이블 출력 유틸리티 재활용

### DummyDataGenerator-HJOY → 테스트 데이터
- 시료 더미 데이터 생성 함수 재활용
- 다양한 주문 상태의 주문 더미 데이터 생성
- `data/` 디렉토리에 JSON 파일로 출력

## 주요 비즈니스 규칙

1. 시스템에 등록된 시료만 주문 가능
2. 주문 승인 시 재고 자동 판단 (수동 조작 불가)
3. 생산 큐는 FIFO — 먼저 들어온 주문부터 생산
4. 생산 완료 시 재고 증가 + 주문 상태 PRODUCING → CONFIRMED 자동 전환
5. 출고 처리 시 주문 수량만큼 재고 차감
6. REJECTED 주문은 모니터링 집계에서 제외
7. 주문번호는 `ORD-YYYYMMDD-NNNN` 형식으로 자동 발급

## 테스트 전략

- 더미 데이터 생성 도구로 초기 데이터 구성 후 시나리오 테스트
- 핵심 시나리오:
  1. 시료 등록 → 주문 접수 → 승인(재고 충분) → 출고
  2. 시료 등록 → 주문 접수 → 승인(재고 부족) → 생산 완료 → 출고
  3. 주문 접수 → 거절
  4. 모니터링 집계 (REJECTED 제외 확인)
  5. 생산 큐 FIFO 순서 확인
  6. 재앱 실행 후 데이터 유지(영속성) 확인

## 평가 주안점 (미션2)

1. **CLAUDE.md, PRD.md 문서 관리** - 이 파일 + PRD.md 유지
2. **Harness 도입** - Claude Code 설정 및 훅 활용
3. **Test** - 핵심 시나리오 검증
4. **CleanCode** - MVC 역할 분리, 단일 책임 원칙
5. **Commit 이력** - 기능 단위의 명확한 커밋 메시지
