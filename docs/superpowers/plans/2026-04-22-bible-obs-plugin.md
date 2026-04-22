# Bible OBS Plugin Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fkmcc_Bible_Plugin을 OBS 공식 플러그인 템플릿으로 이관하고, 파이썬 원본 앱(FkmccBible)의 탭1 UX를 따르는 dock + Bible Verse OBS 소스를 구현.

**Architecture:** 프로세스 싱글톤 `BiblePluginManager`가 단일 상태를 보유. `BibleWidget`(좌: 검색창/구절 리스트/토글, 우: 책 리스트, QSplitter 분할)이 매니저를 조작. `BibleVerseSource`(obs_source_info)가 `verse_changed` 시그널을 구독해 내부 `text_ft2_source_v2`로 렌더.

**Tech Stack:** C++17, Qt 6 (Widgets/Core), SQLite3, OBS 31.1.1, CMake 3.28+, Xcode(macOS).

**Reference Spec:** [docs/superpowers/specs/2026-04-22-bible-obs-plugin-design.md](../specs/2026-04-22-bible-obs-plugin-design.md)

**Python Reference:** [/Users/inseokko/Documents/Fkmcc_Bible](/Users/inseokko/Documents/Fkmcc_Bible) — 탭1의 검색/리스트 UX를 그대로 따른다.

---

## Phase A — 빌드 시스템 이관

### Task A1: 레거시 파일 삭제 & 커밋

**Files:**
- Delete: `src/bible_plugin.cpp`, `src/bible_plugin.h`
- Delete: `azure-pipelines.yml`, `CI/` 전체
- Delete: `lib/` 전체
- Delete: `CMakeLists.txt`

- [ ] **Step 1: 삭제 확인**

```bash
cd /Users/inseokko/Documents/Fkmcc_Bible_Plugin
ls src/bible_plugin.cpp src/bible_plugin.h azure-pipelines.yml CI lib CMakeLists.txt
```

Expected: 모두 존재.

- [ ] **Step 2: 삭제 실행**

```bash
rm -f src/bible_plugin.cpp src/bible_plugin.h azure-pipelines.yml CMakeLists.txt
rm -rf CI lib
```

- [ ] **Step 3: Git 초기화 (repository 아니면)**

```bash
[ -d .git ] || git init
```

- [ ] **Step 4: 커밋**

```bash
git add -A
git commit -m "chore: remove caption plugin remnants and legacy CMakeLists"
```

---

### Task A2: Soniox 템플릿 파일 복사

**Files:**
- Create: `cmake/common/*`, `cmake/macos/*` (복사)
- Create: `build-aux/*` (복사)
- Create: `CMakePresets.json` (복사)

- [ ] **Step 1: 복사 실행**

```bash
cp -R /Users/inseokko/Documents/SonioxCaptionPlugIn/cmake ./cmake
cp -R /Users/inseokko/Documents/SonioxCaptionPlugIn/build-aux ./build-aux
cp /Users/inseokko/Documents/SonioxCaptionPlugIn/CMakePresets.json ./CMakePresets.json
```

- [ ] **Step 2: 확인**

```bash
ls cmake/common/bootstrap.cmake cmake/macos/buildspec.cmake build-aux/run-clang-format CMakePresets.json
```

Expected: 모두 존재.

- [ ] **Step 3: 커밋**

```bash
git add cmake build-aux CMakePresets.json
git commit -m "chore: import OBS plugin template (cmake, build-aux, presets)"
```

---

### Task A3: buildspec.json 생성

**Files:**
- Create: `buildspec.json`

- [ ] **Step 1: 작성**

```bash
cat > buildspec.json <<'EOF'
{
    "dependencies": {
        "obs-studio": {
            "version": "31.1.1",
            "baseUrl": "https://github.com/obsproject/obs-studio/archive/refs/tags",
            "label": "OBS sources",
            "hashes": {
                "macos": "39751f067bacc13d44b116c5138491b5f1391f91516d3d590d874edd21292291"
            }
        },
        "prebuilt": {
            "version": "2025-07-11",
            "baseUrl": "https://github.com/obsproject/obs-deps/releases/download",
            "label": "Pre-Built obs-deps",
            "hashes": {
                "macos": "495687e63383d1a287684b6e2e9bfe246bb8f156fe265926afb1a325af1edd2a"
            }
        },
        "qt6": {
            "version": "2025-07-11",
            "baseUrl": "https://github.com/obsproject/obs-deps/releases/download",
            "label": "Pre-Built Qt6",
            "hashes": {
                "macos": "d3f5f04b6ea486e032530bdf0187cbda9a54e0a49621a4c8ba984c5023998867"
            }
        }
    },
    "platformConfig": {
        "macos": {
            "bundleId": "com.inseokko.obs-bible-plugin"
        }
    },
    "name": "obs-bible-plugin",
    "displayName": "Bible Verses for OBS",
    "version": "0.1.0",
    "author": "inseokko",
    "website": "https://github.com/inseokko/obs-bible-plugin",
    "email": "david.ko@nuwavenow.com"
}
EOF
```

- [ ] **Step 2: 커밋**

```bash
git add buildspec.json
git commit -m "chore: add buildspec.json"
```

---

### Task A4: 루트 CMakeLists.txt 작성

**Files:**
- Create: `CMakeLists.txt`

- [ ] **Step 1: 작성**

```bash
cat > CMakeLists.txt <<'EOF'
cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api" ON)
option(ENABLE_QT "Use Qt functionality" ON)

set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)
include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 REQUIRED COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

find_package(SQLite3 REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SQLite::SQLite3)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
  add_subdirectory(tests)
endif()
EOF
```

- [ ] **Step 2: 커밋**

```bash
git add CMakeLists.txt
git commit -m "chore: write root CMakeLists (Qt6 + SQLite3)"
```

---

### Task A5: locale 파일

**Files:**
- Create: `data/locale/en-US.ini`

- [ ] **Step 1: 작성**

```bash
mkdir -p data/locale
cat > data/locale/en-US.ini <<'EOF'
BibleVerseSource="Bible Verse"
BibleDock="Bible"
BibleSearchPlaceholder="예: 눅 또는 눅1"
BibleSearchButton="검색"
BibleToggleButton="화면 ON/OFF"
BibleDbNotFound="Bible DB not found. Check plugin bundle."
BibleHotkeyNext="Bible: 다음 구절"
BibleHotkeyPrev="Bible: 이전 구절"
BibleHotkeyToggle="Bible: 화면 ON/OFF"
EOF
```

- [ ] **Step 2: 커밋**

```bash
git add data/locale/en-US.ini
git commit -m "feat: add en-US locale strings"
```

---

### Task A6: 스켈레톤 plugin-main.cpp + 첫 빌드

**Files:**
- Create: `src/plugin-main.cpp`
- Create: `tests/CMakeLists.txt` (빈 파일로 시작; `add_subdirectory(tests)` 에러 방지)

- [ ] **Step 1: 스켈레톤 작성**

```bash
cat > src/plugin-main.cpp <<'EOF'
#include <obs-module.h>
#include <util/base.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-bible-plugin", "en-US")

MODULE_EXPORT const char *obs_module_description(void) {
    return "Displays Bible verses in OBS";
}
MODULE_EXPORT const char *obs_module_name(void) {
    return "Bible";
}

bool obs_module_load(void) {
    blog(LOG_INFO, "[obs-bible-plugin] module_load");
    return true;
}
void obs_module_unload(void) {
    blog(LOG_INFO, "[obs-bible-plugin] module_unload");
}
EOF
```

- [ ] **Step 2: tests 디렉토리 초기화**

```bash
mkdir -p tests
cat > tests/CMakeLists.txt <<'EOF'
# Tests added in Phase B
EOF
```

- [ ] **Step 3: 구성**

```bash
cmake --preset macos
```

Expected: `.deps/`에 OBS·obs-deps·qt6 자동 다운로드, Configure 완료.

- [ ] **Step 4: 빌드**

```bash
cmake --build --preset macos
```

Expected: `build_macos/RelWithDebInfo/obs-bible-plugin.plugin` 생성.

- [ ] **Step 5: 번들 검증**

```bash
ls build_macos/RelWithDebInfo/obs-bible-plugin.plugin/Contents/Resources/bible.db
```

Expected: 파일 존재.

- [ ] **Step 6: 설치 + OBS 기동 확인**

```bash
cp -R build_macos/RelWithDebInfo/obs-bible-plugin.plugin \
  ~/Library/Application\ Support/obs-studio/plugins/
open -a OBS
```

OBS 로그(Help → Log Files → View Current Log)에 `[obs-bible-plugin] module_load` 확인.

- [ ] **Step 7: 커밋**

```bash
git add src/plugin-main.cpp tests/CMakeLists.txt
git commit -m "feat: add skeleton plugin-main and tests scaffold"
```

---

## Phase B — 순수 함수 (TDD)

### Task B1: make_search_pattern 구현 + 테스트

**Files:**
- Create: `src/BibleSearch.h`
- Create: `src/BibleSearch.cpp`
- Create: `tests/test_make_search_pattern.cpp`
- Modify: `CMakeLists.txt` (target_sources)
- Modify: `tests/CMakeLists.txt`

- [ ] **Step 1: 테스트 먼저 작성 (TDD: 실패부터)**

```bash
cat > tests/test_make_search_pattern.cpp <<'EOF'
#include "../src/BibleSearch.h"
#include <cstdio>

#define EXPECT(cond) do { if (!(cond)) { \
    fprintf(stderr, "FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
    return 1; \
}} while (0)

int main() {
    // 빈 입력 → 빈 문자열
    EXPECT(make_search_pattern("").isEmpty());
    EXPECT(make_search_pattern("   ").isEmpty());

    // 숫자 없는 키워드 → "...%"
    EXPECT(make_search_pattern("눅") == "눅%");
    EXPECT(make_search_pattern("요일") == "요일%");

    // 숫자 있는 키워드 → "...:%"
    EXPECT(make_search_pattern("눅1") == "눅1:%");
    EXPECT(make_search_pattern("눅10") == "눅10:%");
    EXPECT(make_search_pattern("요일3") == "요일3:%");

    // 앞뒤 공백 trim
    EXPECT(make_search_pattern("  눅1 ") == "눅1:%");

    // LIKE 특수문자 이스케이프 (\, %, _)
    EXPECT(make_search_pattern("눅%") == "눅\\%%");
    EXPECT(make_search_pattern("눅_") == "눅\\_%");
    EXPECT(make_search_pattern("눅\\") == "눅\\\\%");

    printf("make_search_pattern: all tests passed\n");
    return 0;
}
EOF
```

- [ ] **Step 2: 헤더 작성**

```bash
cat > src/BibleSearch.h <<'EOF'
#pragma once
#include <QString>

// 파이썬 원본(model.py:search)과 동일 규칙:
//   키워드에 숫자 있으면 "{kw}:%", 없으면 "{kw}%"
// 빈 입력(공백만 포함) → 빈 QString 반환 (호출측은 검색 skip)
// LIKE 특수문자(%, _, \)는 `\` prefix로 이스케이프 → 호출측 SQL은 ESCAPE '\' 지정
QString make_search_pattern(const QString &raw);
EOF
```

- [ ] **Step 3: 구현**

```bash
cat > src/BibleSearch.cpp <<'EOF'
#include "BibleSearch.h"

QString make_search_pattern(const QString &raw)
{
    QString s = raw.trimmed();
    if (s.isEmpty()) return {};

    bool has_digit = false;
    QString escaped;
    escaped.reserve(s.size() * 2);
    for (QChar c : s) {
        if (c.isDigit()) has_digit = true;
        if (c == '\\' || c == '%' || c == '_')
            escaped.append('\\');
        escaped.append(c);
    }
    escaped.append(has_digit ? ":%" : "%");
    return escaped;
}
EOF
```

- [ ] **Step 4: tests/CMakeLists.txt 작성**

Overwrite `tests/CMakeLists.txt`:

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(test_make_search_pattern
  test_make_search_pattern.cpp
  ../src/BibleSearch.cpp
)
target_include_directories(test_make_search_pattern PRIVATE ../src)
target_link_libraries(test_make_search_pattern PRIVATE Qt6::Core)
set_target_properties(test_make_search_pattern PROPERTIES AUTOMOC OFF)

add_custom_target(run_tests
  COMMAND test_make_search_pattern
  DEPENDS test_make_search_pattern
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
```

- [ ] **Step 5: 루트 CMakeLists의 target_sources에 추가**

Edit `CMakeLists.txt`: `target_sources` 블록 교체.

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/BibleSearch.cpp
)
```

- [ ] **Step 6: 빌드 + 테스트 실행**

```bash
cmake --preset macos
cmake --build --preset macos --target test_make_search_pattern
./build_macos/tests/RelWithDebInfo/test_make_search_pattern
```

Expected: `make_search_pattern: all tests passed`, exit 0.

- [ ] **Step 7: 커밋**

```bash
git add src/BibleSearch.h src/BibleSearch.cpp tests/test_make_search_pattern.cpp \
        tests/CMakeLists.txt CMakeLists.txt
git commit -m "feat(search): add make_search_pattern with LIKE escape + tests"
```

---

### Task B2: BibleSource (search / get_books)

**Files:**
- Create: `src/BibleSource.h`, `src/BibleSource.cpp` (기존 동일 파일 덮어쓰기)
- Create: `tests/fixture_bible.sql`, `tests/test_bible_source.cpp`
- Modify: `CMakeLists.txt`, `tests/CMakeLists.txt`

- [ ] **Step 1: 기존 파일 덮어쓰기 전 확인**

```bash
ls src/BibleSource.h src/BibleSource.cpp
```

Expected: 모두 존재 (Phase A1에서 지우지 않음).

- [ ] **Step 2: 헤더 교체**

```bash
cat > src/BibleSource.h <<'EOF'
#ifndef BIBLE_SOURCE_H
#define BIBLE_SOURCE_H

#include <QString>
#include <vector>
#include <sqlite3.h>

struct BookRow {
    QString name;   // "누가복음"
    QString abbr;   // "눅"
};

struct VerseRow {
    QString verse_ref;  // "눅1:10"
    QString content;
    QString book;       // abbr
    int chapter = 0;
    int verse   = 0;
};

class BibleSource {
public:
    BibleSource();
    ~BibleSource();

    bool initialize(const QString &dbPath);
    bool initializeDefault();                  // obs_module_file("bible.db")

    std::vector<BookRow>  get_books()  const;
    std::vector<VerseRow> search(const QString &like_pattern) const;

private:
    sqlite3 *db = nullptr;
    QString  dbPath;
};

#endif
EOF
```

- [ ] **Step 3: 구현 교체**

```bash
cat > src/BibleSource.cpp <<'EOF'
#include "BibleSource.h"
#include <QFileInfo>
#include <obs-module.h>
#include <util/base.h>

BibleSource::BibleSource() : db(nullptr) {}
BibleSource::~BibleSource() {
    if (db) sqlite3_close(db);
}

bool BibleSource::initialize(const QString &path) {
    dbPath = path;
    QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        blog(LOG_ERROR, "[obs-bible-plugin] Bible DB not found: %s",
             path.toUtf8().constData());
        return false;
    }
    int rc = sqlite3_open(path.toLocal8Bit().constData(), &db);
    if (rc != SQLITE_OK) {
        blog(LOG_ERROR, "[obs-bible-plugin] sqlite3_open failed: %s",
             sqlite3_errmsg(db));
        if (db) { sqlite3_close(db); db = nullptr; }
        return false;
    }
    sqlite3_stmt *s = nullptr;
    const char *chk = "SELECT name FROM sqlite_master WHERE type='table' AND name IN ('bible','bible_books');";
    int n = 0;
    if (sqlite3_prepare_v2(db, chk, -1, &s, nullptr) == SQLITE_OK) {
        while (sqlite3_step(s) == SQLITE_ROW) ++n;
        sqlite3_finalize(s);
    }
    if (n < 2) {
        blog(LOG_ERROR, "[obs-bible-plugin] bible schema invalid (%d/2 tables)", n);
        sqlite3_close(db); db = nullptr;
        return false;
    }
    return true;
}

bool BibleSource::initializeDefault() {
    char *p = obs_module_file("bible.db");
    if (!p) {
        blog(LOG_ERROR, "[obs-bible-plugin] obs_module_file('bible.db') returned null");
        return false;
    }
    QString s = QString::fromUtf8(p);
    bfree(p);
    return initialize(s);
}

std::vector<BookRow> BibleSource::get_books() const {
    std::vector<BookRow> list;
    if (!db) return list;
    sqlite3_stmt *s = nullptr;
    const char *sql = "SELECT name, abbr FROM bible_books ORDER BY id;";
    if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) != SQLITE_OK) {
        blog(LOG_WARNING, "[obs-bible-plugin] get_books prepare failed: %s",
             sqlite3_errmsg(db));
        return list;
    }
    while (sqlite3_step(s) == SQLITE_ROW) {
        BookRow r;
        const unsigned char *n = sqlite3_column_text(s, 0);
        const unsigned char *a = sqlite3_column_text(s, 1);
        r.name = n ? QString::fromUtf8(reinterpret_cast<const char*>(n)) : QString();
        r.abbr = a ? QString::fromUtf8(reinterpret_cast<const char*>(a)) : QString();
        list.emplace_back(std::move(r));
    }
    sqlite3_finalize(s);
    return list;
}

std::vector<VerseRow> BibleSource::search(const QString &like_pattern) const {
    std::vector<VerseRow> list;
    if (!db) return list;
    if (like_pattern.isEmpty()) return list;

    const char *sql =
        "SELECT verse_ref, content, book, chapter, verse "
        "FROM bible "
        "WHERE verse_ref LIKE ? ESCAPE '\\' "
        "ORDER BY (SELECT id FROM bible_books WHERE abbr = bible.book), "
        "         chapter, verse;";
    sqlite3_stmt *s = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) != SQLITE_OK) {
        blog(LOG_WARNING, "[obs-bible-plugin] search prepare failed: %s",
             sqlite3_errmsg(db));
        return list;
    }
    QByteArray pat_utf8 = like_pattern.toUtf8();
    sqlite3_bind_text(s, 1, pat_utf8.constData(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(s) == SQLITE_ROW) {
        VerseRow r;
        const unsigned char *vr = sqlite3_column_text(s, 0);
        const unsigned char *c  = sqlite3_column_text(s, 1);
        const unsigned char *b  = sqlite3_column_text(s, 2);
        r.verse_ref = vr ? QString::fromUtf8(reinterpret_cast<const char*>(vr)) : QString();
        r.content   = c  ? QString::fromUtf8(reinterpret_cast<const char*>(c))  : QString();
        r.book      = b  ? QString::fromUtf8(reinterpret_cast<const char*>(b))  : QString();
        r.chapter   = sqlite3_column_int(s, 3);
        r.verse     = sqlite3_column_int(s, 4);
        list.emplace_back(std::move(r));
    }
    sqlite3_finalize(s);
    return list;
}
EOF
```

- [ ] **Step 4: 픽스처 SQL 작성**

```bash
cat > tests/fixture_bible.sql <<'EOF'
CREATE TABLE bible_books (id INTEGER PRIMARY KEY, name TEXT, abbr TEXT, eng TEXT, testament TEXT);
CREATE TABLE bible (id INTEGER PRIMARY KEY, verse_ref TEXT, content TEXT, book TEXT, chapter INTEGER, verse INTEGER);

INSERT INTO bible_books VALUES (1, '창세기',   '창',   'Genesis', 'OT');
INSERT INTO bible_books VALUES (42,'누가복음', '눅',   'Luke',    'NT');
INSERT INTO bible_books VALUES (43,'요한복음', '요',   'John',    'NT');
INSERT INTO bible_books VALUES (62,'요한일서', '요일', '1 John',  'NT');

INSERT INTO bible VALUES (1,  '창1:1',  '태초에 하나님이 천지를 창조하시니라', '창', 1, 1);
INSERT INTO bible VALUES (2,  '창1:2',  '땅이 혼돈하고 공허하며',              '창', 1, 2);
INSERT INTO bible VALUES (3,  '눅1:1',  '우리 중에 이루어진 사실에 대하여',    '눅', 1, 1);
INSERT INTO bible VALUES (4,  '눅1:10', '모든 백성은 그 분향하는 시간에',      '눅', 1, 10);
INSERT INTO bible VALUES (5,  '눅10:1', '이 후에 주께서 따로 칠십 인을',       '눅', 10, 1);
INSERT INTO bible VALUES (6,  '요3:16', '하나님이 세상을 이처럼 사랑하사',     '요', 3, 16);
INSERT INTO bible VALUES (7,  '요일1:1','태초부터 있는 생명의 말씀에 관하여',  '요일', 1, 1);
EOF
```

- [ ] **Step 5: 테스트 작성**

```bash
cat > tests/test_bible_source.cpp <<'EOF'
#include "../src/BibleSource.h"
#include "../src/BibleSearch.h"
#include <QFile>
#include <QTemporaryFile>
#include <sqlite3.h>
#include <cstdio>

#define EXPECT(cond) do { if (!(cond)) { \
    fprintf(stderr, "FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
    return 1; \
}} while (0)

static bool exec_sql_file(sqlite3 *db, const char *path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QByteArray sql = f.readAll();
    char *err = nullptr;
    int rc = sqlite3_exec(db, sql.constData(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite3_exec failed: %s\n", err ? err : "?");
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    const char *fixture = argc > 1 ? argv[1] : "fixture_bible.sql";

    QTemporaryFile tmp; EXPECT(tmp.open()); tmp.close();
    QString path = tmp.fileName();

    sqlite3 *raw = nullptr;
    EXPECT(sqlite3_open(path.toLocal8Bit().constData(), &raw) == SQLITE_OK);
    EXPECT(exec_sql_file(raw, fixture));
    sqlite3_close(raw);

    BibleSource src;
    EXPECT(src.initialize(path));

    // get_books: 정경 순서(id ASC)
    {
        auto books = src.get_books();
        EXPECT(books.size() == 4);
        EXPECT(books[0].name == "창세기" && books[0].abbr == "창");
        EXPECT(books[1].name == "누가복음");  // id=42 < 43
        EXPECT(books[3].name == "요한일서");
    }

    // "눅" → 눅1:1, 눅1:10, 눅10:1 (3개)
    {
        auto r = src.search(make_search_pattern("눅"));
        EXPECT(r.size() == 3);
        EXPECT(r[0].verse_ref == "눅1:1");
        EXPECT(r[2].verse_ref == "눅10:1");
    }
    // "눅1" → 눅1:1, 눅1:10만 (눅10:1 제외 — :가 구분)
    {
        auto r = src.search(make_search_pattern("눅1"));
        EXPECT(r.size() == 2);
        EXPECT(r[0].verse_ref == "눅1:1");
        EXPECT(r[1].verse_ref == "눅1:10");
    }
    // "눅10" → 눅10:1
    {
        auto r = src.search(make_search_pattern("눅10"));
        EXPECT(r.size() == 1);
        EXPECT(r[0].verse_ref == "눅10:1");
    }
    // "요" → 요3:16, 요일1:1 둘 다
    {
        auto r = src.search(make_search_pattern("요"));
        EXPECT(r.size() == 2);
    }
    // "요1" → 요1:* 없음, 요일1:*는 "요1:%" 패턴에 매칭 안 함 (prefix가 "요1:"인 verse_ref 없음)
    {
        auto r = src.search(make_search_pattern("요1"));
        EXPECT(r.empty());
    }
    // "요일1" → 요일1:1
    {
        auto r = src.search(make_search_pattern("요일1"));
        EXPECT(r.size() == 1);
        EXPECT(r[0].verse_ref == "요일1:1");
    }
    // "xyz" → 빈 결과
    {
        auto r = src.search(make_search_pattern("xyz"));
        EXPECT(r.empty());
    }
    // 빈 입력 → 빈 결과 (search가 즉시 return)
    {
        auto r = src.search(QString());
        EXPECT(r.empty());
    }
    printf("bible_source: all tests passed\n");
    return 0;
}
EOF
```

- [ ] **Step 6: tests/CMakeLists.txt 확장**

Overwrite `tests/CMakeLists.txt`:

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(SQLite3 REQUIRED)
find_package(libobs REQUIRED)

add_executable(test_make_search_pattern
  test_make_search_pattern.cpp
  ../src/BibleSearch.cpp
)
target_include_directories(test_make_search_pattern PRIVATE ../src)
target_link_libraries(test_make_search_pattern PRIVATE Qt6::Core)
set_target_properties(test_make_search_pattern PROPERTIES AUTOMOC OFF)

add_executable(test_bible_source
  test_bible_source.cpp
  ../src/BibleSource.cpp
  ../src/BibleSearch.cpp
)
target_include_directories(test_bible_source PRIVATE ../src)
target_link_libraries(test_bible_source PRIVATE Qt6::Core SQLite::SQLite3 OBS::libobs)
set_target_properties(test_bible_source PROPERTIES AUTOMOC OFF)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fixture_bible.sql
               ${CMAKE_CURRENT_BINARY_DIR}/fixture_bible.sql COPYONLY)

add_custom_target(run_tests
  COMMAND test_make_search_pattern
  COMMAND test_bible_source ${CMAKE_CURRENT_BINARY_DIR}/fixture_bible.sql
  DEPENDS test_make_search_pattern test_bible_source
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
```

- [ ] **Step 7: 루트 CMakeLists 업데이트**

Edit `CMakeLists.txt`의 `target_sources`:

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/BibleSearch.cpp
  src/BibleSource.cpp
)
```

- [ ] **Step 8: 빌드 + 테스트**

```bash
cmake --build --preset macos --target test_bible_source
./build_macos/tests/RelWithDebInfo/test_bible_source \
  build_macos/tests/fixture_bible.sql
```

Expected: `bible_source: all tests passed`, exit 0.

- [ ] **Step 9: 커밋**

```bash
git add src/BibleSource.h src/BibleSource.cpp tests/fixture_bible.sql \
        tests/test_bible_source.cpp tests/CMakeLists.txt CMakeLists.txt
git commit -m "feat(db): add BibleSource::search and get_books with tests"
```

---

## Phase C — BiblePluginManager 싱글톤

### Task C1: BiblePluginSettings (축소)

**Files:**
- Modify: `src/BiblePluginSettings.h`
- Modify: `src/BiblePluginSettings.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 헤더 교체**

```bash
cat > src/BiblePluginSettings.h <<'EOF'
#ifndef BIBLE_PLUGIN_SETTINGS_H
#define BIBLE_PLUGIN_SETTINGS_H

#include <QString>
#include <obs-data.h>

struct BiblePluginSettings {
    QString last_query;
    int     last_index   = -1;
    bool    last_visible = true;
};

void save_BiblePluginSettings_to_config(obs_data_t *save_data, const BiblePluginSettings &s);
BiblePluginSettings load_BiblePluginSettings_from_config(obs_data_t *save_data);

#endif
EOF
```

- [ ] **Step 2: 구현 교체**

```bash
cat > src/BiblePluginSettings.cpp <<'EOF'
#include "BiblePluginSettings.h"

static const char *KEY_OBJ          = "bible_plugin";
static const char *KEY_LAST_QUERY   = "last_query";
static const char *KEY_LAST_INDEX   = "last_index";
static const char *KEY_LAST_VISIBLE = "last_visible";

void save_BiblePluginSettings_to_config(obs_data_t *save_data, const BiblePluginSettings &s) {
    if (!save_data) return;
    obs_data_t *obj = obs_data_create();
    obs_data_set_string(obj, KEY_LAST_QUERY, s.last_query.toUtf8().constData());
    obs_data_set_int   (obj, KEY_LAST_INDEX, s.last_index);
    obs_data_set_bool  (obj, KEY_LAST_VISIBLE, s.last_visible);
    obs_data_set_obj(save_data, KEY_OBJ, obj);
    obs_data_release(obj);
}

BiblePluginSettings load_BiblePluginSettings_from_config(obs_data_t *save_data) {
    BiblePluginSettings s;
    if (!save_data) return s;
    obs_data_t *obj = obs_data_get_obj(save_data, KEY_OBJ);
    if (!obj) return s;
    obs_data_set_default_string(obj, KEY_LAST_QUERY, "");
    obs_data_set_default_int   (obj, KEY_LAST_INDEX, -1);
    obs_data_set_default_bool  (obj, KEY_LAST_VISIBLE, true);
    s.last_query   = QString::fromUtf8(obs_data_get_string(obj, KEY_LAST_QUERY));
    s.last_index   = (int) obs_data_get_int(obj, KEY_LAST_INDEX);
    s.last_visible = obs_data_get_bool(obj, KEY_LAST_VISIBLE);
    obs_data_release(obj);
    return s;
}
EOF
```

- [ ] **Step 3: CMakeLists 업데이트**

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/BibleSearch.cpp
  src/BiblePluginSettings.cpp
  src/BibleSource.cpp
)
```

- [ ] **Step 4: 빌드**

```bash
cmake --build --preset macos
```

Expected: 성공.

- [ ] **Step 5: 커밋**

```bash
git add src/BiblePluginSettings.h src/BiblePluginSettings.cpp CMakeLists.txt
git commit -m "feat: reduce BiblePluginSettings to persistence fields"
```

---

### Task C2: BiblePluginManager 싱글톤

**Files:**
- Create: `src/BiblePluginManager.h` (기존 덮어쓰기)
- Create: `src/BiblePluginManager.cpp` (기존 덮어쓰기)
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 기존 BibleResultHandler 제거** (spec에서 정리)

```bash
rm -f src/BibleResultHandler.h src/BibleResultHandler.cpp
```

- [ ] **Step 2: 매니저 헤더 작성**

```bash
cat > src/BiblePluginManager.h <<'EOF'
#ifndef BIBLE_PLUGIN_MANAGER_H
#define BIBLE_PLUGIN_MANAGER_H

#include <QObject>
#include <QString>
#include <vector>

#include "BibleSource.h"
#include "BiblePluginSettings.h"

class BiblePluginManager : public QObject {
    Q_OBJECT
public:
    static BiblePluginManager *instance();

    void initialize();
    void shutdown();

    Q_INVOKABLE void apply_search(const QString &query);
    Q_INVOKABLE void select_index(int i);
    Q_INVOKABLE void step(int delta);
    Q_INVOKABLE void set_visible(bool v);

    const std::vector<VerseRow> &filtered_list() const { return filtered; }
    std::vector<BookRow> get_books() const { return bible_source.get_books(); }
    int  current_index() const { return cur_index; }
    bool visible()       const { return is_visible; }
    QString current_query() const { return last_query; }

    void import_settings(const BiblePluginSettings &s);
    BiblePluginSettings export_settings() const;

signals:
    void list_changed();
    void verse_changed(QString reference, QString body, bool visible);

private:
    BiblePluginManager();
    ~BiblePluginManager() override;
    void emit_current_verse();

    BibleSource bible_source;
    std::vector<VerseRow> filtered;
    QString last_query;
    int     cur_index = -1;
    bool    is_visible = true;
    bool    initialized = false;
};

#endif
EOF
```

- [ ] **Step 3: 매니저 구현 작성**

```bash
cat > src/BiblePluginManager.cpp <<'EOF'
#include "BiblePluginManager.h"
#include "BibleSearch.h"
#include <util/base.h>

static BiblePluginManager *g_instance = nullptr;

BiblePluginManager *BiblePluginManager::instance() {
    if (!g_instance) g_instance = new BiblePluginManager();
    return g_instance;
}

BiblePluginManager::BiblePluginManager() = default;
BiblePluginManager::~BiblePluginManager() = default;

void BiblePluginManager::initialize() {
    if (initialized) return;
    if (!bible_source.initializeDefault()) {
        blog(LOG_ERROR, "[obs-bible-plugin] BibleSource initializeDefault failed");
    }
    initialized = true;
}

void BiblePluginManager::shutdown() {}

void BiblePluginManager::apply_search(const QString &query) {
    last_query = query;
    filtered.clear();
    cur_index = -1;

    QString pattern = make_search_pattern(query);
    if (!pattern.isEmpty()) {
        filtered = bible_source.search(pattern);
    }

    emit list_changed();
    emit verse_changed(QString(), QString(), is_visible);
}

void BiblePluginManager::select_index(int i) {
    if (i < 0 || i >= (int)filtered.size()) return;
    cur_index = i;
    emit_current_verse();
}

void BiblePluginManager::step(int delta) {
    if (filtered.empty()) return;
    int next;
    if (cur_index < 0) {
        next = (delta > 0) ? 0 : (int)filtered.size() - 1;
    } else {
        next = cur_index + delta;
        if (next < 0 || next >= (int)filtered.size()) return;
    }
    cur_index = next;
    emit_current_verse();
}

void BiblePluginManager::set_visible(bool v) {
    if (is_visible == v) return;
    is_visible = v;
    emit_current_verse();
}

void BiblePluginManager::emit_current_verse() {
    if (cur_index < 0 || cur_index >= (int)filtered.size()) {
        emit verse_changed(QString(), QString(), is_visible);
        return;
    }
    const VerseRow &r = filtered[cur_index];
    emit verse_changed(r.verse_ref, r.content, is_visible);
}

void BiblePluginManager::import_settings(const BiblePluginSettings &s) {
    is_visible = s.last_visible;
    if (!s.last_query.isEmpty()) {
        apply_search(s.last_query);
        if (s.last_index >= 0 && s.last_index < (int)filtered.size())
            select_index(s.last_index);
    }
}

BiblePluginSettings BiblePluginManager::export_settings() const {
    BiblePluginSettings s;
    s.last_query   = last_query;
    s.last_index   = cur_index;
    s.last_visible = is_visible;
    return s;
}
EOF
```

- [ ] **Step 4: CMakeLists 업데이트**

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/BibleSearch.cpp
  src/BiblePluginManager.cpp
  src/BiblePluginSettings.cpp
  src/BibleSource.cpp
)
```

- [ ] **Step 5: 빌드**

```bash
cmake --build --preset macos
```

Expected: 성공 (AUTOMOC이 Q_OBJECT 처리).

- [ ] **Step 6: 커밋**

```bash
git add src/BiblePluginManager.h src/BiblePluginManager.cpp CMakeLists.txt
git rm -f src/BibleResultHandler.h src/BibleResultHandler.cpp 2>/dev/null || true
git add -u
git commit -m "feat: add BiblePluginManager singleton (search/select/step/visible)"
```

---

## Phase D — Dock UI

### Task D1: BibleWidget (좌/우 2패널)

**Files:**
- Create: `src/ui/BibleWidget.h`, `src/ui/BibleWidget.cpp` (기존 덮어쓰기)
- Delete: 기존 `src/ui/BibleSettingsWidget.{h,cpp}`, `src/ui/BibleDock.{h,cpp}` (재작성 전)
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 기존 UI 파일 제거**

```bash
rm -f src/ui/BibleWidget.h src/ui/BibleWidget.cpp \
      src/ui/BibleSettingsWidget.h src/ui/BibleSettingsWidget.cpp \
      src/ui/BibleDock.h src/ui/BibleDock.cpp
```

- [ ] **Step 2: BibleWidget 헤더**

```bash
mkdir -p src/ui
cat > src/ui/BibleWidget.h <<'EOF'
#pragma once
#include <QWidget>

class QLineEdit;
class QPushButton;
class QListView;
class QStandardItemModel;
class QModelIndex;

class BibleWidget : public QWidget {
    Q_OBJECT
public:
    explicit BibleWidget(QWidget *parent = nullptr);
    ~BibleWidget() override;

private slots:
    void onSearchTriggered();
    void onBookClicked(const QModelIndex &index);
    void onVerseCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void onToggleVisibleClicked();

    // manager → UI
    void onListChanged();
    void onVerseChanged(QString reference, QString body, bool visible);

private:
    void populateBookList();
    void refreshVerseList();
    void syncSelectionFromManager();

    // 좌측
    QLineEdit          *searchEdit;
    QPushButton        *searchBtn;
    QListView          *verseView;
    QStandardItemModel *verseModel;
    QPushButton        *toggleBtn;

    // 우측
    QListView          *bookView;
    QStandardItemModel *bookModel;

    bool suppress_selection_feedback = false;
};
EOF
```

- [ ] **Step 3: BibleWidget 구현**

```bash
cat > src/ui/BibleWidget.cpp <<'EOF'
#include "BibleWidget.h"
#include "../BiblePluginManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <obs-module.h>

BibleWidget::BibleWidget(QWidget *parent)
    : QWidget(parent),
      searchEdit(new QLineEdit),
      searchBtn(new QPushButton),
      verseView(new QListView),
      verseModel(new QStandardItemModel(this)),
      toggleBtn(new QPushButton),
      bookView(new QListView),
      bookModel(new QStandardItemModel(this)) {
    searchEdit->setPlaceholderText(obs_module_text("BibleSearchPlaceholder"));
    searchBtn->setText(obs_module_text("BibleSearchButton"));
    toggleBtn->setText(obs_module_text("BibleToggleButton"));

    verseView->setModel(verseModel);
    verseView->setSelectionMode(QAbstractItemView::SingleSelection);
    verseView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    bookView->setModel(bookModel);
    bookView->setSelectionMode(QAbstractItemView::SingleSelection);
    bookView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 좌측 컨테이너
    auto *left = new QWidget(this);
    auto *leftLayout = new QVBoxLayout(left);
    auto *searchRow  = new QHBoxLayout;
    searchRow->addWidget(searchEdit, 1);
    searchRow->addWidget(searchBtn);
    leftLayout->addLayout(searchRow);
    leftLayout->addWidget(verseView, 1);
    leftLayout->addWidget(toggleBtn);

    // 우측 컨테이너 (책 리스트만)
    auto *right = new QWidget(this);
    auto *rightLayout = new QVBoxLayout(right);
    rightLayout->addWidget(bookView, 1);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(left);
    splitter->addWidget(right);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    auto *main = new QVBoxLayout(this);
    main->addWidget(splitter, 1);

    // 시그널 연결
    connect(searchEdit, &QLineEdit::returnPressed, this, &BibleWidget::onSearchTriggered);
    connect(searchBtn,  &QPushButton::clicked,     this, &BibleWidget::onSearchTriggered);
    connect(bookView,   &QListView::clicked,       this, &BibleWidget::onBookClicked);
    connect(toggleBtn,  &QPushButton::clicked,     this, &BibleWidget::onToggleVisibleClicked);
    connect(verseView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &BibleWidget::onVerseCurrentChanged);

    auto *mgr = BiblePluginManager::instance();
    connect(mgr, &BiblePluginManager::list_changed,
            this, &BibleWidget::onListChanged);
    connect(mgr, &BiblePluginManager::verse_changed,
            this, &BibleWidget::onVerseChanged);

    populateBookList();
    searchEdit->setText(mgr->current_query());
    refreshVerseList();
    syncSelectionFromManager();
}

BibleWidget::~BibleWidget() = default;

void BibleWidget::populateBookList() {
    bookModel->clear();
    for (const auto &b : BiblePluginManager::instance()->get_books()) {
        auto *it = new QStandardItem(b.name + ":" + b.abbr);
        it->setData(b.abbr, Qt::UserRole + 1);
        bookModel->appendRow(it);
    }
}

void BibleWidget::onSearchTriggered() {
    BiblePluginManager::instance()->apply_search(searchEdit->text());
}

void BibleWidget::onBookClicked(const QModelIndex &index) {
    if (!index.isValid()) return;
    QString abbr = index.data(Qt::UserRole + 1).toString();
    searchEdit->setText(abbr);
    BiblePluginManager::instance()->apply_search(abbr);
}

void BibleWidget::onVerseCurrentChanged(const QModelIndex &current, const QModelIndex &) {
    if (!current.isValid()) return;
    suppress_selection_feedback = true;
    BiblePluginManager::instance()->select_index(current.row());
    suppress_selection_feedback = false;
}

void BibleWidget::onToggleVisibleClicked() {
    auto *m = BiblePluginManager::instance();
    m->set_visible(!m->visible());
}

void BibleWidget::onListChanged() {
    refreshVerseList();
}

void BibleWidget::onVerseChanged(QString, QString, bool) {
    if (suppress_selection_feedback) return;
    syncSelectionFromManager();
}

void BibleWidget::refreshVerseList() {
    verseModel->clear();
    const auto &list = BiblePluginManager::instance()->filtered_list();
    for (const auto &r : list) {
        QString line = r.verse_ref + "  " + r.content;
        verseModel->appendRow(new QStandardItem(line));
    }
}

void BibleWidget::syncSelectionFromManager() {
    int i = BiblePluginManager::instance()->current_index();
    if (i < 0 || i >= verseModel->rowCount()) {
        verseView->clearSelection();
        return;
    }
    QModelIndex idx = verseModel->index(i, 0);
    verseView->setCurrentIndex(idx);
    verseView->scrollTo(idx);
}
EOF
```

- [ ] **Step 4: CMakeLists 업데이트**

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/BibleSearch.cpp
  src/BiblePluginManager.cpp
  src/BiblePluginSettings.cpp
  src/BibleSource.cpp
  src/ui/BibleWidget.cpp
)
```

- [ ] **Step 5: 빌드**

```bash
cmake --build --preset macos
```

Expected: 성공.

- [ ] **Step 6: 커밋**

```bash
git add src/ui/BibleWidget.h src/ui/BibleWidget.cpp CMakeLists.txt
git rm -f src/ui/BibleSettingsWidget.h src/ui/BibleSettingsWidget.cpp \
          src/ui/BibleDock.h src/ui/BibleDock.cpp 2>/dev/null || true
git add -u
git commit -m "feat(ui): BibleWidget with split-panel verse list + book list"
```

---

### Task D2: BibleDock

**Files:**
- Create: `src/ui/BibleDock.h`, `src/ui/BibleDock.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 헤더 작성**

```bash
cat > src/ui/BibleDock.h <<'EOF'
#pragma once
#include <QDockWidget>

class BibleWidget;

class BibleDock : public QDockWidget {
    Q_OBJECT
public:
    explicit BibleDock(QWidget *parent = nullptr);
    ~BibleDock() override;
private:
    BibleWidget *bibleWidget;
};
EOF
```

- [ ] **Step 2: 구현 작성**

```bash
cat > src/ui/BibleDock.cpp <<'EOF'
#include "BibleDock.h"
#include "BibleWidget.h"
#include <obs-module.h>

BibleDock::BibleDock(QWidget *parent)
    : QDockWidget(obs_module_text("BibleDock"), parent),
      bibleWidget(new BibleWidget(this)) {
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setObjectName("bible_dock");
    setWidget(bibleWidget);
}

BibleDock::~BibleDock() = default;
EOF
```

- [ ] **Step 3: CMakeLists 업데이트**

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/BibleSearch.cpp
  src/BiblePluginManager.cpp
  src/BiblePluginSettings.cpp
  src/BibleSource.cpp
  src/ui/BibleDock.cpp
  src/ui/BibleWidget.cpp
)
```

- [ ] **Step 4: 빌드**

```bash
cmake --build --preset macos
```

Expected: 성공.

- [ ] **Step 5: 커밋**

```bash
git add src/ui/BibleDock.h src/ui/BibleDock.cpp CMakeLists.txt
git commit -m "feat(ui): BibleDock wrapper"
```

---

### Task D3: plugin-main.cpp — dock + hotkey + save/load

**Files:**
- Modify: `src/plugin-main.cpp`

- [ ] **Step 1: 전체 교체**

```bash
cat > src/plugin-main.cpp <<'EOF'
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/base.h>

#include <QMainWindow>
#include <QMetaObject>

#include "BiblePluginManager.h"
#include "BiblePluginSettings.h"
#include "ui/BibleDock.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-bible-plugin", "en-US")

static BibleDock *g_dock = nullptr;

static obs_hotkey_id g_hk_next   = OBS_INVALID_HOTKEY_ID;
static obs_hotkey_id g_hk_prev   = OBS_INVALID_HOTKEY_ID;
static obs_hotkey_id g_hk_toggle = OBS_INVALID_HOTKEY_ID;

static void hk_cb(void *, obs_hotkey_id id, obs_hotkey_t *, bool pressed) {
    if (!pressed) return;
    auto *mgr = BiblePluginManager::instance();
    if (id == g_hk_next) {
        QMetaObject::invokeMethod(mgr, "step", Qt::QueuedConnection, Q_ARG(int, +1));
    } else if (id == g_hk_prev) {
        QMetaObject::invokeMethod(mgr, "step", Qt::QueuedConnection, Q_ARG(int, -1));
    } else if (id == g_hk_toggle) {
        QMetaObject::invokeMethod(mgr, "set_visible", Qt::QueuedConnection,
                                  Q_ARG(bool, !mgr->visible()));
    }
}

static void register_hotkeys() {
    g_hk_next   = obs_hotkey_register_frontend("bible.next",
        obs_module_text("BibleHotkeyNext"), hk_cb, nullptr);
    g_hk_prev   = obs_hotkey_register_frontend("bible.prev",
        obs_module_text("BibleHotkeyPrev"), hk_cb, nullptr);
    g_hk_toggle = obs_hotkey_register_frontend("bible.toggle",
        obs_module_text("BibleHotkeyToggle"), hk_cb, nullptr);
}

static void unregister_hotkeys() {
    if (g_hk_next   != OBS_INVALID_HOTKEY_ID) obs_hotkey_unregister(g_hk_next);
    if (g_hk_prev   != OBS_INVALID_HOTKEY_ID) obs_hotkey_unregister(g_hk_prev);
    if (g_hk_toggle != OBS_INVALID_HOTKEY_ID) obs_hotkey_unregister(g_hk_toggle);
    g_hk_next = g_hk_prev = g_hk_toggle = OBS_INVALID_HOTKEY_ID;
}

static void setup_dock() {
    if (g_dock) return;
    g_dock = new BibleDock();
    obs_frontend_add_dock_by_id("bible_dock", obs_module_text("BibleDock"), g_dock);
}

static void on_frontend_event(enum obs_frontend_event ev, void *) {
    if (ev == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
        BiblePluginManager::instance()->initialize();
        setup_dock();
    } else if (ev == OBS_FRONTEND_EVENT_EXIT) {
        g_dock = nullptr;
    }
}

static void on_save_or_load(obs_data_t *save_data, bool saving, void *) {
    auto *mgr = BiblePluginManager::instance();
    if (saving) {
        save_BiblePluginSettings_to_config(save_data, mgr->export_settings());
    } else {
        BiblePluginSettings s = load_BiblePluginSettings_from_config(save_data);
        mgr->import_settings(s);
    }
}

MODULE_EXPORT const char *obs_module_description(void) {
    return "Displays Bible verses in OBS";
}
MODULE_EXPORT const char *obs_module_name(void) {
    return "Bible";
}

bool obs_module_load(void) {
    blog(LOG_INFO, "[obs-bible-plugin] module_load");
    register_hotkeys();
    obs_frontend_add_event_callback(on_frontend_event, nullptr);
    obs_frontend_add_save_callback(on_save_or_load, nullptr);
    return true;
}

void obs_module_unload(void) {
    blog(LOG_INFO, "[obs-bible-plugin] module_unload");
    obs_frontend_remove_save_callback(on_save_or_load, nullptr);
    obs_frontend_remove_event_callback(on_frontend_event, nullptr);
    unregister_hotkeys();
    BiblePluginManager::instance()->shutdown();
}
EOF
```

- [ ] **Step 2: 빌드 + 설치**

```bash
cmake --build --preset macos
cp -R build_macos/RelWithDebInfo/obs-bible-plugin.plugin \
  ~/Library/Application\ Support/obs-studio/plugins/
open -a OBS
```

- [ ] **Step 3: 수동 확인**

OBS에서:
- View → Docks → **Bible** 체크. dock 표시 확인.
- 우측 책 리스트에 66권 표시 확인.
- 책 리스트에서 "누가복음:눅" 클릭 → 검색창에 `눅`, 좌측 리스트에 누가복음 전체 구절.
- 검색창에 `눅1` 타이핑 후 Enter → 누가복음 1장만 표시.
- 좌측 리스트에서 화살표 ↑/↓ 이동 가능.
- "화면 ON/OFF" 버튼 클릭 가능 (아직 씬에 보이는 효과는 없음 — 다음 Phase).

- [ ] **Step 4: 커밋**

```bash
git add src/plugin-main.cpp
git commit -m "feat: wire plugin-main with dock, hotkeys, save/load"
```

---

## Phase E — BibleVerseSource

### Task E1: obs_source_info 등록 + 생성/파괴 + text proxy

**Files:**
- Create: `src/BibleVerseSource.h`, `src/BibleVerseSource.cpp`
- Modify: `src/plugin-main.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 헤더**

```bash
cat > src/BibleVerseSource.h <<'EOF'
#pragma once

void bible_verse_source_register();
EOF
```

- [ ] **Step 2: 구현**

```bash
cat > src/BibleVerseSource.cpp <<'EOF'
#include "BibleVerseSource.h"
#include "BiblePluginManager.h"

#include <obs-module.h>
#include <util/base.h>
#include <QMetaObject>
#include <QString>
#include <string>

namespace {

struct bible_verse_source {
    obs_source_t *src = nullptr;
    obs_source_t *text_source = nullptr;
    QMetaObject::Connection verse_conn;
};

static void apply_text(obs_source_t *ts, const QString &line) {
    if (!ts) return;
    obs_data_t *d = obs_data_create();
    obs_data_set_string(d, "text", line.toUtf8().constData());
    obs_source_update(ts, d);
    obs_data_release(d);
}

static QString make_display(const QString &ref, const QString &body, bool visible) {
    if (!visible) return {};
    if (ref.isEmpty() && body.isEmpty()) return {};
    if (ref.isEmpty()) return body;
    if (body.isEmpty()) return ref;
    return ref + "  " + body;
}

static obs_source_t *create_text_source(obs_data_t *src_settings) {
    obs_data_t *d = obs_data_create();
    obs_data_t *font = obs_data_get_obj(src_settings, "font");
    if (!font) {
        font = obs_data_create();
        obs_data_set_string(font, "face",  "Apple SD Gothic Neo");
        obs_data_set_string(font, "style", "Regular");
        obs_data_set_int   (font, "size",  48);
        obs_data_set_int   (font, "flags", 0);
    }
    obs_data_set_obj(d, "font", font);
    obs_data_release(font);

    obs_data_set_int (d, "color1", obs_data_get_int(src_settings, "color1"));
    obs_data_set_int (d, "color2", obs_data_get_int(src_settings, "color1"));
    obs_data_set_bool(d, "outline", obs_data_get_bool(src_settings, "outline"));
    obs_data_set_bool(d, "drop_shadow", obs_data_get_bool(src_settings, "drop_shadow"));
    obs_data_set_bool(d, "word_wrap", obs_data_get_bool(src_settings, "word_wrap"));
    obs_data_set_int (d, "custom_width", obs_data_get_int(src_settings, "custom_width"));
    obs_data_set_string(d, "text", "");

    obs_source_t *ts = obs_source_create_private(
        "text_ft2_source_v2", "obs-bible-plugin.text_source", d);
    obs_data_release(d);
    return ts;
}

static const char *bvs_get_name(void *) {
    return obs_module_text("BibleVerseSource");
}

static void *bvs_create(obs_data_t *settings, obs_source_t *source) {
    auto *ctx = new bible_verse_source();
    ctx->src = source;
    ctx->text_source = create_text_source(settings);

    auto *mgr = BiblePluginManager::instance();
    ctx->verse_conn = QObject::connect(
        mgr, &BiblePluginManager::verse_changed,
        mgr,  // receiver (QObject 필요, disconnect는 Connection 객체로)
        [ctx](QString ref, QString body, bool visible) {
            apply_text(ctx->text_source, make_display(ref, body, visible));
        },
        Qt::QueuedConnection);

    // 초기 상태 1회 반영
    int i = mgr->current_index();
    const auto &lst = mgr->filtered_list();
    if (i >= 0 && i < (int)lst.size()) {
        const VerseRow &r = lst[i];
        apply_text(ctx->text_source, make_display(r.verse_ref, r.content, mgr->visible()));
    } else {
        apply_text(ctx->text_source, {});
    }
    return ctx;
}

static void bvs_destroy(void *data) {
    auto *ctx = static_cast<bible_verse_source *>(data);
    if (!ctx) return;
    QObject::disconnect(ctx->verse_conn);
    if (ctx->text_source) obs_source_release(ctx->text_source);
    delete ctx;
}

static uint32_t bvs_get_width(void *data) {
    auto *ctx = static_cast<bible_verse_source *>(data);
    return (ctx && ctx->text_source) ? obs_source_get_width(ctx->text_source) : 0;
}
static uint32_t bvs_get_height(void *data) {
    auto *ctx = static_cast<bible_verse_source *>(data);
    return (ctx && ctx->text_source) ? obs_source_get_height(ctx->text_source) : 0;
}
static void bvs_video_render(void *data, gs_effect_t *) {
    auto *ctx = static_cast<bible_verse_source *>(data);
    if (ctx && ctx->text_source) obs_source_video_render(ctx->text_source);
}

static obs_properties_t *bvs_get_properties(void *) {
    obs_properties_t *p = obs_properties_create();
    obs_properties_add_font(p, "font", "Font");
    obs_properties_add_color(p, "color1", "Color");
    obs_properties_add_bool (p, "outline", "Outline");
    obs_properties_add_bool (p, "drop_shadow", "Drop Shadow");
    obs_properties_add_bool (p, "word_wrap", "Word Wrap");
    obs_properties_add_int  (p, "custom_width", "Custom Width", 0, 8192, 1);
    return p;
}

static void bvs_get_defaults(obs_data_t *d) {
    obs_data_t *font = obs_data_create();
    obs_data_set_default_string(font, "face",  "Apple SD Gothic Neo");
    obs_data_set_default_string(font, "style", "Regular");
    obs_data_set_default_int   (font, "size",  48);
    obs_data_set_default_int   (font, "flags", 0);
    obs_data_set_default_obj(d, "font", font);
    obs_data_release(font);
    obs_data_set_default_int (d, "color1", 0xFFFFFFFF);
    obs_data_set_default_bool(d, "outline", false);
    obs_data_set_default_bool(d, "drop_shadow", false);
    obs_data_set_default_bool(d, "word_wrap", false);
    obs_data_set_default_int (d, "custom_width", 0);
}

static void bvs_update(void *data, obs_data_t *settings) {
    auto *ctx = static_cast<bible_verse_source *>(data);
    if (!ctx || !ctx->text_source) return;

    obs_data_t *cur = obs_source_get_settings(ctx->text_source);
    std::string preserve = obs_data_get_string(cur, "text");
    obs_data_release(cur);

    obs_data_t *d = obs_data_create();
    obs_data_t *font = obs_data_get_obj(settings, "font");
    if (font) {
        obs_data_set_obj(d, "font", font);
        obs_data_release(font);
    }
    obs_data_set_int (d, "color1", obs_data_get_int(settings, "color1"));
    obs_data_set_int (d, "color2", obs_data_get_int(settings, "color1"));
    obs_data_set_bool(d, "outline", obs_data_get_bool(settings, "outline"));
    obs_data_set_bool(d, "drop_shadow", obs_data_get_bool(settings, "drop_shadow"));
    obs_data_set_bool(d, "word_wrap", obs_data_get_bool(settings, "word_wrap"));
    obs_data_set_int (d, "custom_width", obs_data_get_int(settings, "custom_width"));
    obs_data_set_string(d, "text", preserve.c_str());
    obs_source_update(ctx->text_source, d);
    obs_data_release(d);
}

static struct obs_source_info g_info = {};

} // anonymous namespace

void bible_verse_source_register() {
    g_info.id             = "obs-bible-plugin.bible_verse";
    g_info.type           = OBS_SOURCE_TYPE_INPUT;
    g_info.output_flags   = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
    g_info.icon_type      = OBS_ICON_TYPE_TEXT;
    g_info.get_name       = bvs_get_name;
    g_info.create         = bvs_create;
    g_info.destroy        = bvs_destroy;
    g_info.get_width      = bvs_get_width;
    g_info.get_height     = bvs_get_height;
    g_info.video_render   = bvs_video_render;
    g_info.get_properties = bvs_get_properties;
    g_info.get_defaults   = bvs_get_defaults;
    g_info.update         = bvs_update;
    obs_register_source(&g_info);
}
EOF
```

- [ ] **Step 3: plugin-main.cpp에 등록 호출**

Edit `src/plugin-main.cpp`: `#include "ui/BibleDock.h"` 아래에 추가:

```cpp
#include "BibleVerseSource.h"
```

`obs_module_load`의 `register_hotkeys();` 직전에 추가:

```cpp
    bible_verse_source_register();
```

- [ ] **Step 4: CMakeLists 업데이트**

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/BibleSearch.cpp
  src/BiblePluginManager.cpp
  src/BiblePluginSettings.cpp
  src/BibleSource.cpp
  src/BibleVerseSource.cpp
  src/ui/BibleDock.cpp
  src/ui/BibleWidget.cpp
)
```

- [ ] **Step 5: 빌드 + 설치**

```bash
cmake --build --preset macos
cp -R build_macos/RelWithDebInfo/obs-bible-plugin.plugin \
  ~/Library/Application\ Support/obs-studio/plugins/
open -a OBS
```

- [ ] **Step 6: 수동 확인**

OBS에서:
- 씬 "+" → "Bible Verse" 추가. 속성 패널에 Font/Color/Outline/Drop Shadow/Word Wrap/Custom Width 노출.
- dock에서 "누가복음:눅" 선택 → 검색 → 첫 구절 클릭 → 씬 텍스트에 `눅1:1  우리 중에...` 표시.
- 화살표 ↓ 이동 → 다음 구절로 갱신.
- "화면 ON/OFF" 버튼 → 빈 문자열로 바뀜, 다시 누르면 복원.
- 소스 속성 Font 변경 → 즉시 렌더 반영.

- [ ] **Step 7: 커밋**

```bash
git add src/BibleVerseSource.h src/BibleVerseSource.cpp src/plugin-main.cpp CMakeLists.txt
git commit -m "feat(source): add BibleVerseSource proxying text_ft2_source_v2"
```

---

## Phase F — 수락 테스트

### Task F1: 기능 체크리스트 실행

**Files:** (없음 — 수동)

- [ ] **Step 1: 빌드 + 설치 확인**

```bash
cmake --build --preset macos
cp -R build_macos/RelWithDebInfo/obs-bible-plugin.plugin \
  ~/Library/Application\ Support/obs-studio/plugins/
ls ~/Library/Application\ Support/obs-studio/plugins/obs-bible-plugin.plugin/Contents/Resources/bible.db
```

Expected: `bible.db` 존재.

- [ ] **Step 2: 로그 확인**

OBS 로그에 `[obs-bible-plugin] module_load` 확인. `Bible DB not found` 없어야 함.

- [ ] **Step 3: 기능 체크리스트**

| 항목 | Pass? |
|---|---|
| View → Docks → Bible 로 dock 노출 | ☐ |
| 우측 책 리스트에 66권 `{name}:{abbr}` 표시, 정경 순서 | ☐ |
| "누가복음:눅" 클릭 → 검색창 `눅`, 좌측 누가복음 전체 구절 | ☐ |
| 검색창 `눅1` + Enter → 누가복음 1장만 | ☐ |
| 검색창 `눅10` + Enter → 10장만 (1장 미포함) | ☐ |
| 검색창 `눅100` + Enter → 빈 리스트 | ☐ |
| 검색창 빈 문자열 Enter → 빈 리스트 + 화면 지움 | ☐ |
| 좌측 리스트 마우스 클릭 → 씬 텍스트 `{verse_ref}  {content}` | ☐ |
| 좌측 리스트 ↑/↓ 키 → 선택 이동 + 씬 텍스트 갱신 | ☐ |
| 설정 → 단축키에서 `bible.next/prev/toggle` 바인딩 | ☐ |
| 핫키 누르면 dock 포커스 없어도 이동/토글 동작 | ☐ |
| "화면 ON/OFF" 버튼 → 씬 빈 문자열 ↔ 구절 | ☐ |
| 두 씬에 Bible Verse 소스 두고 씬 전환 → 같은 구절 유지 | ☐ |
| 소스 속성 Font/Color/Outline 변경 → 즉시 반영 | ☐ |
| OBS 재시작 → 마지막 검색어 + 인덱스 + 가시성 복원 | ☐ |

- [ ] **Step 4: 장애 내성 (옵션)**

번들 내부 `Resources/bible.db`를 다른 이름으로 rename → OBS 재시작 → dock은 뜨지만 책 리스트와 구절 리스트가 비어 있음, 크래시 없음, 로그에 `Bible DB not found`. 확인 후 이름 복원.

- [ ] **Step 5: 커밋 (문서 정리 있으면)**

```bash
git status
# HANDOFF.md 등 정리 있으면 커밋, 아니면 skip
```

---

## Self-review

**1. Spec coverage**:
- 렌더 방식 A (자체 소스): Task E1 ✓
- 검색 문법 (`{kw}:%` / `{kw}%`): Task B1 (pattern) + B2 (SQL) ✓
- 책 리스트 패널: Task D1 (BibleWidget 우측) ✓
- 책 클릭 → 검색 주입 + 자동 검색: Task D1 (`onBookClicked`) ✓
- Enter/버튼 트리거: Task D1 (`returnPressed` + `clicked`) ✓
- 좌우 버튼 제거, 화면 ON/OFF만: Task D1 (버튼 1개) ✓
- 키보드 ↑/↓ 이동: Task D1 (`currentChanged` 시그널 기본 동작) ✓
- 한 줄 포맷 `{verse_ref}  {content}`: Task D1 `refreshVerseList` ✓
- 단일 상태 공유: Task C2 (singleton) + E1 (시그널 구독) ✓
- 스타일 Soniox 수준: Task E1 (properties) ✓
- 핫키 3개: Task D3 ✓
- 빌드 템플릿 이관: Task A1–A6 ✓
- 저장/로드: Task C1 + D3 ✓
- DB 경로 `bible.db`: Task B2 (`initializeDefault`) ✓
- 에러 처리: Task B2 (initialize 로그/false 반환) ✓
- 수락 테스트: Task F1 ✓
- Out of scope (OT/NT 탭, 찬송가, PPT): 의도 미포함 ✓

**2. Placeholder scan**: "TBD"/"TODO"/"handle edge cases" 없음. 모든 코드 블록은 컴파일 가능한 완전한 형태.

**3. Type consistency**:
- `VerseRow` 필드 (`verse_ref`/`content`/`book`/`chapter`/`verse`) — B2에서 정의, C2/D1/E1에서 사용 ✓
- `BookRow` (`name`/`abbr`) — B2에서 정의, C2/D1에서 사용 ✓
- `make_search_pattern(QString) → QString` — B1 정의, C2 호출 일치 ✓
- `BiblePluginManager` 메서드 — 헤더 C2 선언, `QMetaObject::invokeMethod`의 이름·타입 D3 호출 일치 (`step(int)`, `set_visible(bool)`) ✓
- save/load 함수 — C1 선언/정의, D3 호출 일치 ✓
- `BibleVerseSource` 속성 키(`font`/`color1`/`outline` 등) — E1 내부 일관 ✓

수정 없음.
