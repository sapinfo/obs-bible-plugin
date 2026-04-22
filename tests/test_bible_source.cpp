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
    // "요1" → 요1:* 없음, 요일1:*는 "요1:%" 패턴에 매칭 안 함
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
