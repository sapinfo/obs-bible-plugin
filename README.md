# OBS Bible Plugin

OBS Studio에서 한국어 성경 구절을 실시간으로 검색해 화면에 띄우는 플러그인.

교회 예배 / 설교 방송 중에 운영자가 왼쪽 dock 패널에서 구절을 고르면, 씬에 배치한 Bible Verse 소스가 그 구절을 즉시 렌더링합니다.

## 기능

- **Bible Dock** — OBS 메인 윈도우에 도킹되는 Qt 패널
  - 좌측: 검색창(Enter/버튼) + 구절 리스트(마우스 클릭 / 키보드 ↑↓로 이동) + 화면 ON/OFF 토글
  - 우측: 66권 책 리스트 (`누가복음:눅` 형식). 클릭하면 검색창에 약어가 자동 입력되고 즉시 검색
- **Bible Verse OBS 소스** — 씬에 추가해 현재 선택 구절을 화면에 렌더
  - 내장 `text_ft2_source_v2`에 프록시하여 Font / Color / Outline / Drop Shadow / Word Wrap / Custom Width 속성 지원
  - 여러 씬에 동일 소스 배치 가능, 씬 전환해도 현재 구절 유지
- **글로벌 단축키** — OBS 설정 → 단축키에서 바인딩
  - `Bible: 다음 구절` / `Bible: 이전 구절` / `Bible: 화면 ON/OFF`
- **상태 영속성** — 마지막 검색어·선택 인덱스·가시성이 OBS 재시작 후 자동 복원

## 검색 문법

| 입력 | 결과 |
|---|---|
| `눅` | 누가복음 전체 구절 |
| `눅1` | 누가복음 1장만 (콜론 구분자 덕에 `눅10:*`은 제외됨) |
| `눅10` | 누가복음 10장만 |
| `요` | 요한복음 + 요한1·2·3서 통합 |
| `요일1` | 요한1서 1장만 |

책 리스트에서 책을 클릭하면 약어가 검색창에 들어가고, 장 번호를 바로 이어 타이핑해서 Enter를 치면 됩니다.

## 설치

[Releases 페이지](https://github.com/sapinfo/obs-bible-plugin/releases)에서 플랫폼별 에셋 다운로드.

### macOS (Apple Silicon)
`obs-bible-plugin-*-macos-arm64.pkg` 다운로드 → 더블클릭 설치 (OBS 종료 후) → OBS 재기동 → View → Docks → Bible 체크.

### macOS (Intel)
`obs-bible-plugin-*-macos-x86_64.pkg`

### Windows x64
`obs-bible-plugin-*-windows-x64.zip` 다운로드 → 압축 해제 → 내부 `obs-plugins/` 폴더의 내용을 `C:\Program Files\obs-studio\obs-plugins\`에 병합.

### Linux (Ubuntu 24.04 x86_64)
```bash
sudo dpkg -i obs-bible-plugin-*-x86_64-linux-gnu.deb
```

## 요구 사항

- OBS Studio 31.x 이상 (32.x에서 검증)
- macOS 12.0+ / Windows 10+ / Ubuntu 24.04+

## 소스에서 빌드 (macOS 기준)

OBS 공식 플러그인 템플릿 기반이라 한 줄 커맨드로 의존성 자동 수신 + 빌드됩니다.

```bash
cmake --preset macos
cmake --build --preset macos
cp -R build_macos/RelWithDebInfo/obs-bible-plugin.plugin \
  ~/Library/Application\ Support/obs-studio/plugins/
```

처음 `cmake --preset macos` 실행 시 `.deps/` 아래로 OBS 31.1.1 소스, obs-deps, Qt6 prebuilt가 자동 다운로드됩니다 (수 GB, 5~10분).

로컬 유닛 테스트(macOS만) 실행:
```bash
cmake --preset macos -DBUILD_TESTS=ON
cmake --build --preset macos --target test_make_search_pattern --target test_bible_source
./build_macos/tests/RelWithDebInfo/test_make_search_pattern
./build_macos/tests/RelWithDebInfo/test_bible_source build_macos/tests/fixture_bible.sql
```

## 기술 스택

- C++17, Qt 6 (Widgets·Core), OBS 31.1.1
- SQLite3 amalgamation 정적 링크 (`vendor/sqlite3/`)
- 성경 DB: 개역개정판 (플러그인 번들의 `Resources/bible.db`)

## 관련 프로젝트

이 플러그인의 UX는 동일 저자의 **파이썬 데스크탑 앱 FkmccBible**(PySide6 / SQLite3)의 탭1 검색 동작을 그대로 따릅니다. 예배 준비용 PPT 생성·찬송가 파일 관리는 파이썬 앱 쪽, 예배 중 OBS 송출은 이 플러그인 쪽으로 역할이 나뉘어 있습니다.

## 알려진 제한

- 단일 번역본만 지원 (번역본 전환 기능 없음)
- 다국어 검색 미지원 (한글 abbr 기반 검색만)
- 다중 구절 동시 표시 / 즐겨찾기 미지원

## 라이선스

원본 코드 베이스의 라이선스는 [LICENSE](LICENSE) 참조. 벤더링된 SQLite3 amalgamation은 Public Domain (자세한 내용은 [vendor/sqlite3/README.md](vendor/sqlite3/README.md)).
