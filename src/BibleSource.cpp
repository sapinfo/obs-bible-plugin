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
