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
