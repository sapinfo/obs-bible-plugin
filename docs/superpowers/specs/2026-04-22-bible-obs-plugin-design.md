# Bible OBS Plugin — 설계안

**문서 일자**: 2026-04-22
**대상 플러그인**: `obs-bible-plugin`
**플랫폼 우선순위**: macOS arm64 (1순위), macOS x86_64 / Windows x64 / Ubuntu (추후)

---

## 1. 목적

OBS Studio에서 한국어 성경 구절을 실시간으로 검색·선택해 씬에 오버레이 표시하는 플러그인. 예배·설교 중 운영자가 dock 패널에서 구절을 고르고, 좌우 버튼 또는 핫키로 이동하면 OBS 씬에 즉시 반영되어야 한다.

## 2. 사용자 흐름

1. OBS를 켜면 좌측/하단에 **Bible** dock이 나타난다.
2. 씬에 "+ 추가 → Bible Verse" 소스를 배치한다 (원하는 개수, 원하는 씬).
3. dock 우측 책 리스트(`누가복음:눅` 형식)에서 원하는 책을 클릭하면 검색창에 abbr(`눅`)가 자동 입력되고, 리스트가 해당 책 전체 구절로 갱신된다.
4. 검색창에서 abbr 뒤에 장 번호를 추가로 타이핑하고 **Enter**를 누르면(예: `눅1`) 해당 장만 필터된다. 또는 **검색** 버튼 클릭.
5. 구절 리스트에서 마우스 클릭 또는 **키보드 ↑/↓**로 선택을 이동하면, 현재 선택된 구절이 모든 씬의 Bible Verse 소스에 즉시 표시된다.
6. "화면 ON/OFF" 토글 또는 핫키로 텍스트 표시/비표시 전환.
7. 씬을 전환해도 현재 선택 구절은 유지된다. OBS를 재시작해도 마지막 상태가 복원된다.

파이썬 원본 [FkmccBible 앱](/Users/inseokko/Documents/Fkmcc_Bible)의 탭1 UX와 동일한 조작감. "찬송가 열기"·"PPT 생성"·"Keynote 관리" 기능은 OBS 플러그인 범위 밖.

## 3. 주요 결정 사항

| 항목 | 결정 | 근거 |
|---|---|---|
| 렌더 방식 | 플러그인이 자체 OBS Source 제공 | 폰트·색·외곽선 등을 플러그인이 통제, 사용자 설정 실수 여지 제거 |
| 검색 문법 | `verse_ref LIKE` 패턴. 키워드에 숫자 있으면 `{kw}:%`, 없으면 `{kw}%` | 파이썬 원본([FkmccBible/model.py](/Users/inseokko/Documents/Fkmcc_Bible/model.py))과 동일. `:` 구분자가 `눅1:%` vs `눅10:*`을 자연스럽게 구분 |
| 검색 트리거 | **Enter** 또는 **검색 버튼**. 타이핑 중 자동 검색 없음 | 예측 가능, 원본과 동일 UX |
| 책 리스트 패널 | dock 우측에 전체 책 리스트(`{name}:{abbr}`). 탭 없음. | 원본 UX 준수, 빠른 책 선택. OT/NT 구분은 불필요(사용자 결정) |
| 좌우 이동 | **키보드 ↑/↓**로 QListView 선택 이동. 전용 버튼 없음 | dock UI 간결. `currentRowChanged` 시그널로 자동 처리 |
| 경계 동작 | 리스트 처음/끝에서 ↑/↓ 이동 시 Qt 기본 동작 유지(=이동 안 함) | 단순 |
| 화면 포맷 | 한 줄 "참조 + 본문" (예: `눅 1:10  ...`) — `verse_ref + " " + content` | 원본 `on_update_list`와 동일 포맷 |
| 상태 범위 | 프로세스 전역 단일 상태 (모든 소스 인스턴스 공유) | 씬 전환해도 현재 구절 유지 |
| 스타일 노출 | Soniox 수준 (Font dialog / Color1 / Outline / Drop Shadow / Word Wrap / Custom Width) | 기존 레퍼런스 존재, 예배 사용에 충분 |
| 핫키 | `bible.next` / `bible.prev` / `bible.toggle` 3개 (OBS 설정에서 키 바인딩) | dock 포커스 없어도 원격·키보드 운영 가능 |

## 4. 아키텍처

```
┌───────────────────────────────────┬────────────────────┐
│  BibleDock (QDockWidget, 좌)      │  책 리스트 패널(우)  │  ◀ BibleWidget
│  ┌─────────────────────────────┐  │  ┌──────────────┐  │     (QSplitter 내부)
│  │ 검색창 [눅1    ] [검색]     │  │  │ 갈라디아서:갈 │  │
│  ├─────────────────────────────┤  │  │ 고린도전서:고전│  │
│  │ 눅1:1 우리 중에 이루어진... │  │  │ ...          │  │
│  │ 눅1:2 처음부터 목격자와...  │  │  │ 누가복음:눅  │◀── 클릭 →
│  │ ...  (↑/↓ 키로 이동)        │  │  │ ...          │    검색창에 "눅"
│  │ 선택 행이 즉시 화면 반영     │  │  └──────────────┘    주입 + 검색
│  ├─────────────────────────────┤  │                      │
│  │       [화면 ON/OFF]         │  │                      │
│  └─────────────────────────────┘  │                      │
└───────────────────────────────────┴──────────────────────┘
          │
          │  apply_search / select_index / step / set_visible
          ▼
┌──────────────────────────────────────┐
│  BiblePluginManager (QObject 싱글톤)  │
│  - BibleSource (SQLite)              │
│  - 현재 필터 결과 vector<VerseRow>    │
│  - cur_index, is_visible             │
└─────────────┬────────────────────────┘
              │ Qt signal: verse_changed(ref, body, visible)
              │          : list_changed()
              ▼
   ┌─────────────────────────────────┐
   │  Bible Verse Source (obs_source │
   │  _info) N개 — 씬마다 하나씩     │
   │  내부 text_ft2_source_v2에       │
   │  text 속성 주입 (proxy)          │
   └─────────────────────────────────┘
```

## 5. 컴포넌트별 책임

### 5.1 `BiblePluginManager` (QObject, 싱글톤)

```cpp
class BiblePluginManager : public QObject {
    Q_OBJECT
public:
    static BiblePluginManager* instance();

    // 검색·필터
    Q_INVOKABLE void apply_search(const QString &query);
    const std::vector<VerseRow>& filtered_list() const;
    QString current_query() const;

    // 선택·이동 (step은 핫키 전용; dock은 select_index만 사용)
    Q_INVOKABLE void select_index(int i);
    Q_INVOKABLE void step(int delta);
    int  current_index() const;

    // 가시성
    Q_INVOKABLE void set_visible(bool v);
    bool visible() const;

    // 책 리스트 (책 리스트 패널용)
    std::vector<BookRow> get_books() const;

signals:
    void list_changed();
    void verse_changed(QString reference, QString body, bool visible);

private:
    BibleSource bible_source;
    std::vector<VerseRow> filtered;
    QString last_query;
    int cur_index = -1;
    bool is_visible = true;
};

struct BookRow {
    QString name;     // "누가복음"
    QString abbr;     // "눅"
};

struct VerseRow {
    QString verse_ref;   // "눅1:10"
    QString content;
    QString book;        // abbr (DB 원본)
    int chapter;
    int verse;
};
```

### 5.2 `BibleSource` (DB 접근)

```cpp
class BibleSource {
public:
    bool initialize(const QString &dbPath);
    bool initializeDefault();                          // obs_module_file("bible.db")

    // 책 리스트 (우측 패널용). bible_books.id 기준 정렬
    std::vector<BookRow> get_books() const;

    // 검색: 키워드에 숫자 있으면 pattern = keyword+":%", 없으면 keyword+"%"
    // 파이썬 원본 model.py 동작과 동일
    std::vector<VerseRow> search(const QString &keyword) const;
};
```

`search()` SQL:

```sql
SELECT verse_ref, content, book, chapter, verse
FROM bible
WHERE verse_ref LIKE ? ESCAPE '\'
ORDER BY (SELECT id FROM bible_books WHERE abbr = bible.book),
         chapter, verse;
```

- 키워드 바인딩 전에 `%`, `_`, `\` 문자를 `\`로 이스케이프.
- 이스케이프된 키워드 뒤에 "숫자 포함 시 `:%`, 아니면 `%`"를 덧붙여 패턴 완성.
- `ORDER BY` 서브쿼리는 여러 책이 매칭될 때(예: `요` → 요한복음/요한1서/요한2서/요한3서/요엘) 정경 순서를 유지하기 위함.

### 5.3 `make_search_pattern` (순수 함수)

```cpp
// "눅"    → "눅%"
// "눅1"   → "눅1:%"
// "눅1장" → "눅1장:%"  (숫자 포함하면 어디 있든 콜론 suffix)
// ""      → "" (호출측이 검색 자체를 skip)
QString make_search_pattern(const QString &raw);
```

`raw`는 이스케이프 전 원본. 내부에서 `QChar::isDigit()`으로 숫자 유무만 판정 후 이스케이프·접미사를 붙인다.

### 5.4 `BibleDock` / `BibleWidget`

```
┌────────────────────────────────────┬───────────────────┐
│ [검색: 눅          ] [검색]        │ 갈라디아서:갈     │ ← QListView
├────────────────────────────────────┤ 고린도전서:고전   │   (book list)
│ 눅1:1  우리 중에 이루어진 사실에   │ ...               │
│ 눅1:2  처음부터 목격자와 말씀의    │ 누가복음:눅       │
│ 눅1:3  그 모든 일을 근원부터 자세히 │ ...               │
│ ...                                 │                   │
│ (↑/↓ 키로 선택 이동 → 즉시 화면)   │                   │
├────────────────────────────────────┤                   │
│ [화면 ON/OFF]                      │                   │
└────────────────────────────────────┴───────────────────┘
  |<-- verse panel (stretch) -->|<-- book panel (fixed) -->|
                                       (QSplitter로 사용자 리사이즈 가능)
```

- **좌측 상단**: `QLineEdit`(검색창) + `QPushButton`("검색").
  - `QLineEdit::returnPressed` / `QPushButton::clicked` → `manager->apply_search(text)`.
  - **타이핑 중 자동 검색 없음**.
- **좌측 중앙**: `QListView` + `QStandardItemModel`. 한 줄 포맷 `"{verse_ref} {content}"` 예: `눅1:1  우리 중에...`.
  - `QItemSelectionModel::currentChanged` 시그널 → `manager->select_index(row)` → 다른 소스 인스턴스 렌더 갱신. **키보드 ↑/↓ 시 자동 발신**.
- **좌측 하단**: `QPushButton`("화면 ON/OFF") 단 1개. 클릭 시 `manager->set_visible(!visible)`.
- **우측**: `QListView` + `QStandardItemModel`. `BookRow` 66권을 `"{name}:{abbr}"`로 표시.
  - 클릭 시 `searchEdit->setText(abbr)` → `apply_search(abbr)` → 좌측 리스트 갱신.
- `BiblePluginManager::list_changed()` 시그널 → 좌측 모델 재구성.
- `verse_changed()` 시그널 → 좌측 리스트의 선택 하이라이트 sync. 단, 자신이 쏜 선택에 의한 재진입은 guard 플래그로 방지.
- 책 리스트는 생성 시 `manager->get_books()` 1회 조회로 채운다(DB 변경 없는 정적 데이터).

### 5.5 `BibleVerseSource` (신규, `obs_source_info` 등록)

```cpp
struct bible_verse_source {
    obs_source_t *src;
    obs_source_t *text_source;       // 내부 text_ft2_source_v2 (proxy)
    QMetaObject::Connection verse_conn;

    // 스타일 속성 (OBS properties로 노출)
    int font_size;
    std::string font_face, font_style;
    uint32_t color1, color2;
    bool outline, drop_shadow;
    int custom_width;
    bool word_wrap;
};
```

- `create`: Qt 메인 스레드에서 `BiblePluginManager::instance()->verse_changed`에 연결. 현재 상태로 1회 수동 렌더.
- `update`: OBS properties(`obs_data_t`)에서 스타일 읽어 내부 `text_source`의 동일 키에 전달 후 `obs_source_update`.
- `video_render`: 내부 `text_source`를 그리기 (Soniox의 proxy 패턴).
- `get_width/get_height`: 내부 `text_source`의 값 pass-through.
- `get_properties`: Font 다이얼로그 1개 + Color1 + Outline + Drop Shadow + Word Wrap + Custom Width.
- `destroy`: `QObject::disconnect(verse_conn)` 명시적 해제 후 내부 `text_source` release.

### 5.6 `plugin-main.cpp` (엔트리포인트)

- `obs_module_load`
  - `BiblePluginManager::instance()` 생성(싱글톤).
  - `obs_register_source(&bible_verse_source_info)`.
  - `obs_frontend_add_event_callback`, `obs_frontend_add_save_callback` 등록.
  - 매니저의 `register_hotkeys()` 호출.
- `OBS_FRONTEND_EVENT_FINISHED_LOADING`: dock 생성 & `obs_frontend_add_dock_by_id`.
- `OBS_FRONTEND_EVENT_EXIT`: 매니저 해제 (소스 인스턴스는 OBS가 destroy 콜백으로 정리).
- save callback:
  - saving 시 `BiblePluginSettings` + 현재 `last_query/last_index/last_visible` 직렬화.
  - loading 시 역직렬화해서 매니저에 주입.

## 6. 데이터 흐름

### 6.1 검색 → 리스트 갱신

1. 사용자가 검색창에 `눅1` 타이핑 후 **Enter** 또는 **검색** 버튼 클릭.
   또는: 우측 책 리스트에서 `누가복음:눅` 클릭 → `searchEdit->setText("눅")` → 자동으로 `apply_search("눅")` 트리거.
2. `BibleWidget::onSearch()` → `BiblePluginManager::apply_search("눅1")`.
3. `make_search_pattern("눅1")` → `"눅1:%"` (숫자 포함).
4. `BibleSource::search(pattern)` → SQL 실행 → `filtered` 갱신.
5. `last_query = "눅1"`, `cur_index = -1`.
6. `emit list_changed()` → 좌측 리스트 재렌더.
7. `emit verse_changed("", "", visible)` → 모든 Bible Verse 소스 화면 지움.

**패턴 규칙** (`make_search_pattern`):
- 앞뒤 공백 trim, `%/_/\` 이스케이프.
- `QString::contains` + `QChar::isDigit`으로 숫자 유무 판정.
- 숫자 있음 → `{kw}:%`, 없음 → `{kw}%`.
- 빈 입력 → `""` 반환, 호출측은 search skip하고 `filtered.clear()` + 시그널만 emit.

### 6.2 구절 선택 → 화면 반영

1. 마우스 클릭 또는 **키보드 ↑/↓** → `QItemSelectionModel::currentChanged(index)`.
2. `BibleWidget::onCurrentChanged(index)` → `BiblePluginManager::select_index(index.row())`.
3. `cur_index = i`; `VerseRow r = filtered[i]`.
4. `ref = r.verse_ref` (예: `"눅1:10"` — DB 원본 그대로).
5. `body = r.content`.
6. `emit verse_changed(ref, body, is_visible)`.
7. 각 소스는 `visible ? (ref + "  " + body) : ""` 를 내부 text_source에 주입.
   - 출력 예: `눅1:10  ...`.

### 6.3 이동 (핫키 전용)

dock UI에는 이동 버튼 없음. 이동은 **QListView의 기본 키보드 동작**(↑/↓) 또는 **글로벌 핫키**(`bible.next` / `bible.prev`)로만.

`step(delta)` — 핫키에서만 호출:
- `filtered.empty()` → no-op.
- `cur_index == -1`이면 `delta>0`일 때 `select_index(0)`, `delta<0`일 때 `select_index(last)`.
- 아니면 `next = cur_index + delta`, `0 <= next < size`일 때만 `select_index(next)`, 밖이면 no-op.
- 매니저가 `select_index`를 호출하면 `verse_changed` emit 및 dock 리스트 선택도 sync됨.

### 6.4 핫키

- `bible.next` → `step(+1)`
- `bible.prev` → `step(-1)`
- `bible.toggle` → `set_visible(!visible())`
- 핫키 콜백은 OBS 내부 스레드 가능성 있음. `QMetaObject::invokeMethod(manager, "<slot>", Qt::QueuedConnection)`으로 메인 스레드 점프.

### 6.5 저장/로드

`BiblePluginSettings`에 필드 추가:

```cpp
struct BiblePluginSettings {
    // 기존 스타일 기본값
    int fontSize;
    QString fontFamily;
    bool autoScroll;   // (옵션) 리스트가 현재 구절로 자동 스크롤

    // 운영 상태 복원용
    QString last_query;
    int     last_index;
    bool    last_visible;
};
```

저장: `obs_data_t` 서브오브젝트 `"bible_plugin"`에 위 필드 직렬화.
로드: 역직렬화 후 매니저에 주입 — 매니저가 `apply_search(last_query)` → `select_index(last_index)` 순서로 복원.

개별 소스 인스턴스의 **스타일 속성**은 OBS가 기본 `obs_source_save` 경로로 자체 저장하므로 플러그인 관여 불필요.

## 7. 빌드 시스템

OBS 공식 플러그인 템플릿(SonioxCaptionPlugIn 참조)으로 이관.

**복사**: `cmake/` + `build-aux/` + `CMakePresets.json`
**신규**: `buildspec.json` — OBS 31.1.1, obs-deps 2025-07-11, qt6 2025-07-11
**`CMakeLists.txt`**: `ENABLE_FRONTEND_API=ON`, `ENABLE_QT=ON`, SQLite3 추가.

디렉토리(이관 후):

```
Fkmcc_Bible_Plugin/
├── buildspec.json                  # 신규
├── CMakePresets.json               # 신규
├── CMakeLists.txt                  # 재작성
├── cmake/                          # 템플릿 복사
├── build-aux/                      # 템플릿 복사
├── data/
│   ├── bible.db
│   └── locale/en-US.ini            # 신규
├── src/
│   ├── plugin-main.cpp
│   ├── BiblePluginManager.{h,cpp}
│   ├── BiblePluginSettings.{h,cpp}
│   ├── BibleResultHandler.{h,cpp}
│   ├── BibleSource.{h,cpp}
│   ├── BibleVerseSource.{h,cpp}    # 신규
│   └── ui/
│       ├── BibleDock.{h,cpp}
│       ├── BibleWidget.{h,cpp}     # 레이아웃 재작업
│       └── BibleSettingsWidget.{h,cpp}
└── README.md
```

**삭제 대상**:
- `azure-pipelines.yml`, 기존 `CI/` 디렉토리 — 템플릿 CI로 대체
- `lib/caption_stream/` 전체 — 캡션 원본 잔재
- `src/bible_plugin.cpp` / `src/bible_plugin.h` — `plugin-main.cpp`로 통합

**경로 변경**: `obs_module_file("data/bible.db")` → `obs_module_file("bible.db")` (템플릿의 `target_install_resources`가 `data/` 하위를 `Contents/Resources/` 바로 아래로 복사).

**빌드 커맨드**:

```bash
cmake --preset macos
cmake --build --preset macos
cp -R build_macos/RelWithDebInfo/obs-bible-plugin.plugin \
  ~/Library/Application\ Support/obs-studio/plugins/
```

## 8. 에러 처리 & 경계 조건

### 8.1 DB

| 상황 | 처리 |
|---|---|
| `obs_module_file` 실패 / `sqlite3_open` 실패 / 스키마 불일치 | `blog(LOG_ERROR)` + 모든 질의 empty 반환. dock에 "Bible DB not found" 안내 1행. 크래시 없음. |
| 질의 중 SQLite 에러 | `qWarning` + 빈 결과. |

### 8.2 검색

| 입력 | 동작 |
|---|---|
| 빈 문자열 | empty list + `cur_index=-1` + 화면 지움 (SQL 실행 skip) |
| 순수 숫자 `"42"` | `42:%` 매칭 없으므로 empty list |
| 매칭 없음 `"xyz"` | empty list |
| `%`, `_`, `\` 포함 | SQL `ESCAPE '\'`로 안전 처리 — 원문 그대로 매칭 시도 |

### 8.3 이동

- `filtered.empty()` → no-op.
- `cur_index==-1`에서 `step(+1)` → `select_index(0)`.
- `cur_index==-1`에서 `step(-1)` → `select_index(size-1)`.
- 경계에서 `step(±1)` → no-op.
- 새 검색 시 화면 지움(`verse_changed("", "", visible)` emit) — "검색 중" 상태 반영.

### 8.4 소스 수명/스레드

- `create` 시 Qt `connect`, `destroy` 시 명시적 `disconnect` (소스는 C struct).
- 플러그인 종료 시 매니저 파괴로 연결 자동 끊김, 이후 각 source `destroy` 호출.
- 핫키는 OBS 내부 스레드 가능성 → `QMetaObject::invokeMethod(Qt::QueuedConnection)`로 메인 스레드 점프.
- `video_render`는 그래픽 스레드 — 내부 text_source proxy만 (Soniox 동일).

## 9. 테스트 전략

### 9.1 단위 분리

다음 함수는 단독 테스트 가능한 형태로 설계:

- `make_search_pattern(QString) → QString` — 순수 함수 (Qt만 의존).
- `BibleSource::search(keyword)` — in-memory SQLite로 테스트 가능.
- `BibleSource::get_books()` — in-memory SQLite로 테스트 가능.

### 9.2 수동 수락 테스트

**빌드 & 설치**:
- [ ] `cmake --preset macos` / `--build` 성공
- [ ] `obs-bible-plugin.plugin` 번들 생성, `Contents/Resources/bible.db` 존재
- [ ] OBS 재시작 후 로그에 `obs_bible_plugin ... obs_module_load` 출력

**기능**:
- [ ] View → Docks → Bible 로 dock 노출
- [ ] 씬 "+ → Bible Verse" 소스 추가 가능
- [ ] 우측 책 리스트에 66권 표시 (`{name}:{abbr}` 형식), 정경 순서
- [ ] 우측에서 "누가복음:눅" 클릭 → 검색창에 `눅` 자동 입력 + 좌측에 누가복음 전체 구절
- [ ] 검색창에 `눅1` 타이핑 + Enter → 누가복음 1장만 필터. `눅10` → 10장만. `눅100` → 빈 리스트.
- [ ] 검색창 빈 문자열 Enter → 빈 리스트 + 화면 지움
- [ ] 좌측 리스트 마우스 클릭 → 씬 텍스트 즉시 갱신 (`눅1:1  우리 중에...`)
- [ ] 좌측 리스트 포커스 후 **키보드 ↑/↓** → 선택 이동 + 씬 텍스트 즉시 갱신
- [ ] 핫키 바인딩 후 `bible.next`/`bible.prev` 키 누르면 이동 (dock 포커스 없어도)
- [ ] `bible.toggle` 핫키 / "화면 ON/OFF" 버튼 → 씬 텍스트 빈 문자열 / 복원
- [ ] 같은 소스를 두 씬에 두고 씬 전환 시 현재 구절 유지
- [ ] 소스 속성 Font / Color1 / Outline / Drop Shadow 변경 즉시 반영
- [ ] OBS 재시작: 마지막 검색어/인덱스/가시성 복원, 핫키 바인딩 유지

**장애 내성**:
- [ ] `bible.db` 제거 상태 재시작 → 크래시 없음, "DB not found" 안내
- [ ] 씬 컬렉션 전환 후 Bible Verse 소스 추가 → 현재 구절 즉시 반영

### 9.3 비기능

- 창세기 전체(~1,500행) 검색 응답 < 100ms, dock 렌더 < 300ms.
- 1시간 연속 가동 시 메모리 누수 없음 (Activity Monitor 관찰).
- 한글 렌더 기본 폰트 "Apple SD Gothic Neo".

### 9.4 범위

- 1순위: macOS arm64 (현재 개발 환경).
- 2순위: macOS x86_64 / Windows x64 / Ubuntu — CI 빌드만 통과시키고 수동 수락은 추후.

## 10. 범위 밖 (Out of Scope)

- 영문/중국어 등 다국어 검색 (DB 영문 컬럼 `eng` 존재하나 이번 범위 X).
- 구약/신약/찬송가열기 탭 구분 (사용자 결정 — 책 리스트는 단일 리스트로 66권 표시).
- PPT 생성 / 찬송가 열기 / Keynote 파일 관리 (파이썬 원본의 기능이지만 OBS 플러그인 범위 밖).
- 다중 선택 / 여러 구절 동시 표시.
- 즐겨찾기·북마크 기능.
- 여러 번역본 지원 (NKJV, 개역개정 등).
- 본문 복수 줄 분할 표시 (한 줄 포맷 고정).
- 기존 `BibleSettingsWidget`/`BibleResultHandler` 클래스 재사용 — 설계 단순화를 위해 모두 제거, 포맷은 `VerseRow`에서 직접 생성.

## 11. 리스크 & 미정

- **Qt/OBS 버전 호환**: buildspec.json 템플릿의 31.1.1과 Qt 6.x 조합에서 `obs_frontend_add_dock_by_id`가 정상 동작. 이전 API(`obs_frontend_add_dock`)는 deprecated이므로 `_by_id` 사용.
- **스타일 속성 키 이름**: OBS 내장 `text_ft2_source_v2` 기준 (`color1`, `color2`, `outline`, `drop_shadow`, `custom_width`, `word_wrap`, `font`). 이 키 이름은 Soniox가 이미 검증.
- **핫키 `bible.toggle`의 시각적 피드백**: 현재는 dock 버튼 텍스트 변경 없음. 필요 시 후속으로 버튼 상태 싱크 추가.
