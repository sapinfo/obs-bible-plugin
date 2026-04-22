#pragma once
#include <QString>

// 파이썬 원본(model.py:search)과 동일 규칙:
//   키워드에 숫자 있으면 "{kw}:%", 없으면 "{kw}%"
// 빈 입력(공백만 포함) → 빈 QString 반환 (호출측은 검색 skip)
// LIKE 특수문자(%, _, \)는 `\` prefix로 이스케이프 → 호출측 SQL은 ESCAPE '\' 지정
QString make_search_pattern(const QString &raw);
