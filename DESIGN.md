# DummyDataGen — 설계 문서

## 목적

반도체 시료 생산 관리 Console App의 기능 PoC(Proof of Concept) 검증을 위한 더미 데이터 자동 생성 도구.

본 프로그램 자체가 테스트 대상이 아니며, **테스트 대상 프로그램에 공급할 입력 데이터를 파일로 만드는 것**이 전부다.
생성된 파일은 향후 테스트 대상 프로그램이 File I/O로 읽어 들인다.

---

## 도메인 기본 개념

| 용어 | 설명 |
|------|------|
| 시료 (Sample) | 생산·분석 대상이 되는 반도체 시편 단위 |
| 주문 (Order) | 특정 시각에 시료의 생산/처리를 요청하는 행위 |
| 주문 이벤트 | `(시료, 주문 시각)` 쌍을 기본 단위로 하는 데이터 레코드 |

> **핵심 불변식:** 더미 데이터의 모든 레코드는 반드시 하나의 시료와 하나의 주문 시각을 가진다.

---

## 데이터 모델

### 시료 (Sample)

| 필드 | 타입 | 예시 | 비고 |
|------|------|------|------|
| `sample_id` | string | `SMP-00042` | 고유 식별자, 순번 기반 생성 |
| `material` | enum | `Si`, `SiC`, `GaAs`, `GaN` | 반도체 재질 |
| `diameter_mm` | int | `100`, `150`, `200`, `300` | 웨이퍼 직경(mm) |
| `thickness_um` | int | `525`, `650`, `725` | 두께(μm) |
| `process_step` | enum | `BARE`, `OXIDATION`, `LITHOGRAPHY`, `ETCH`, `DEPOSITION`, `CMP`, `INSPECTION` | 현재 공정 단계 |
| `lot_id` | string | `LOT-2026-0001` | 동일 배치 묶음 식별자 |
| `quantity` | int | 1 ~ 25 | 시료 매수 |

### 주문 (Order)

| 필드 | 타입 | 예시 | 비고 |
|------|------|------|------|
| `order_id` | string | `ORD-00001` | 고유 식별자, 순번 기반 생성 |
| `order_time` | datetime | `2026-05-08T09:32:11` | ISO 8601, 초 단위 |
| `requester_id` | string | `ENG-007` | 요청자 ID |
| `priority` | enum | `LOW`, `NORMAL`, `HIGH`, `URGENT` | 처리 우선순위 |
| `due_date` | date | `2026-05-15` | 납기 요구일 |
| `status` | enum | `PENDING`, `IN_PROGRESS`, `COMPLETED`, `CANCELLED` | 초기값 `PENDING` |

### 주문 이벤트 레코드 (Order Event — 출력 단위)

시료와 주문을 결합한 단일 레코드. 출력 파일의 한 행(row)에 대응한다.

```
order_id, order_time, requester_id, priority, due_date, status,
sample_id, lot_id, material, diameter_mm, thickness_um, process_step, quantity
```

---

## 출력 파일 명세

### 파일 형식: CSV (UTF-8, LF)

- 1행: 헤더
- 2행~: 주문 이벤트 레코드 (1행 = 1이벤트)
- 필드 구분: `,`
- 문자열 필드: `"` 쌍따옴표로 감싸지 않음 (값 내부에 콤마 없음이 보장됨)

### 파일명 규칙

```
dummy_YYYYMMDD_HHMMSS_N.csv
```

- `YYYYMMDD_HHMMSS`: 파일 생성 시각
- `N`: 파일 내 레코드 수 (예: `dummy_20260508_093211_1000.csv`)

### 출력 예시

```csv
order_id,order_time,requester_id,priority,due_date,status,sample_id,lot_id,material,diameter_mm,thickness_um,process_step,quantity
ORD-00001,2026-05-08T09:32:11,ENG-007,NORMAL,2026-05-15,PENDING,SMP-00042,LOT-2026-0001,Si,200,725,OXIDATION,5
ORD-00002,2026-05-08T10:05:44,ENG-003,HIGH,2026-05-12,PENDING,SMP-00043,LOT-2026-0001,SiC,150,650,ETCH,3
ORD-00003,2026-05-08T11:17:02,ENG-011,URGENT,2026-05-09,PENDING,SMP-00044,LOT-2026-0002,GaN,100,525,INSPECTION,1
```

---

## 생성 파라미터 (프로그램 실행 시 설정)

| 파라미터 | 기본값 | 설명 |
|---------|--------|------|
| `--count N` | 100 | 생성할 레코드 수 |
| `--seed S` | 현재 시각 | 난수 시드 (재현성 확보) |
| `--start-time T` | `2026-01-01T00:00:00` | 주문 시각 범위 시작 |
| `--end-time T` | 현재 시각 | 주문 시각 범위 끝 |
| `--out-dir PATH` | `./output` | 출력 파일 저장 경로 |

---

## 생성 전략

### 주문 시각 분포

- `[start-time, end-time]` 구간을 균등 분포로 샘플링
- 실제 공정 패턴 모사를 위해 업무 시간(09:00~18:00) 가중치를 선택적으로 적용 가능 (향후 확장)

### 시료 ID / Lot ID 생성

- `sample_id`: `SMP-NNNNN` (5자리, 순번)
- `lot_id`: 일정 확률로 동일 Lot을 여러 주문에 공유 (현실적 데이터 분포 모사)
- Lot당 묶이는 시료 수: 1~10개 (균등 난수)

### 우선순위 분포

| Priority | 비율 |
|----------|------|
| LOW | 10% |
| NORMAL | 60% |
| HIGH | 25% |
| URGENT | 5% |

### 납기일 생성

- `due_date = order_date + delta_days`
- `delta_days`: priority별 범위
  - URGENT: 1~3일
  - HIGH: 3~7일
  - NORMAL: 7~21일
  - LOW: 14~30일

---

## 구현 구조 (C++20, MSVC)

```
DummyDataGen/
├── Project1/
│   ├── DummyDataGen.slnx
│   └── Project1/
│       ├── Project1.vcxproj
│       ├── main.cpp          ← 진입점, 파라미터 파싱
│       ├── Generator.h/.cpp  ← 난수 기반 레코드 생성 로직
│       ├── DataModel.h       ← Sample, Order, OrderEvent 구조체 / enum
│       ├── CsvWriter.h/.cpp  ← 파일 출력
│       └── Config.h          ← 실행 파라미터 구조체
└── output/                   ← 생성된 더미 CSV 파일 저장 위치
```

### 주요 타입

```cpp
// DataModel.h
enum class Material    { Si, SiC, GaAs, GaN };
enum class ProcessStep { BARE, OXIDATION, LITHOGRAPHY, ETCH, DEPOSITION, CMP, INSPECTION };
enum class Priority    { LOW, NORMAL, HIGH, URGENT };
enum class Status      { PENDING, IN_PROGRESS, COMPLETED, CANCELLED };

struct Sample {
    std::string sample_id;
    std::string lot_id;
    Material    material;
    int         diameter_mm;
    int         thickness_um;
    ProcessStep process_step;
    int         quantity;
};

struct Order {
    std::string order_id;
    std::string order_time;   // ISO 8601 string
    std::string requester_id;
    Priority    priority;
    std::string due_date;
    Status      status;
};

struct OrderEvent {
    Order  order;
    Sample sample;
};
```

---

## 향후 확장 고려사항 (현재 구현 범위 밖)

- JSON / 이진 포맷 출력 지원
- 주문 이벤트 스트림 시뮬레이션 (시간 순서 보장 출력)
- 시나리오 기반 생성 (특정 공정 병목, 긴급 주문 급증 등)
- 테스트 대상 프로그램과의 직접 파이프 연동 (현재는 파일 경유)
