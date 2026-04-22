#include "BibleSource.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <obs-module.h>

BibleSource::BibleSource() : db(nullptr) {}

BibleSource::~BibleSource() {
    if (db) {
        sqlite3_close(db);
    }
}

bool BibleSource::initialize(const QString &dbPath) {
    this->dbPath = dbPath;
    QFileInfo info(dbPath);
    if (!info.exists() || !info.isFile()) {
        qWarning() << "Bible DB file not found:" << dbPath;
        return false;
    }

    int rc = sqlite3_open(dbPath.toLocal8Bit().constData(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Cannot open bible database:" << sqlite3_errmsg(db);
        sqlite3_close(db);
        db = nullptr;
        return false;
    }

    // Optionally, check that the expected tables exist
    const char *check_sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='bible';";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            qWarning() << "Expected table 'bible' not found in database.";
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            db = nullptr;
            return false;
        }
        sqlite3_finalize(stmt);
    } else {
        qWarning() << "Failed to check for bible table:" << sqlite3_errmsg(db);
        sqlite3_close(db);
        db = nullptr;
        return false;
    }

    return true;
}

bool BibleSource::initializeDefault() {
    char *module_path = obs_module_file("data/bible.db");
    if (!module_path) {
        qWarning() << "obs_module_file failed; cannot resolve default bible.db path";
        return false;
    }

    QString db_file = QString::fromUtf8(module_path);
    bfree(module_path);
    return initialize(db_file);
}

std::vector<QString> BibleSource::get_book_list() const {
    std::vector<QString> list;
    if (!db) return list;

    const char *sql = "SELECT name FROM bible_books ORDER BY name;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        qWarning() << "Failed to prepare book list query:" << sqlite3_errmsg(db);
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        if (text) {
            list.emplace_back(QString::fromUtf8(reinterpret_cast<const char*>(text)));
        }
    }
    sqlite3_finalize(stmt);
    return list;
}

int BibleSource::get_chapter_count(const QString &bookName) const {
    if (!db) return 0;

    const char *sql =
        "SELECT MAX(b.chapter) "
        "FROM bible b "
        "JOIN bible_books bb ON bb.abbr = b.book "
        "WHERE bb.name = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        qWarning() << "Failed to prepare chapter count query:" << sqlite3_errmsg(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, bookName.toLocal8Bit().constData(), -1, SQLITE_TRANSIENT);

    int maxChapter = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        maxChapter = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return maxChapter;
}

std::vector<std::pair<int, QString>> BibleSource::get_verses(const QString &bookName, int chapter, int offset, int limit) const {
    std::vector<std::pair<int, QString>> list;
    if (!db) return list;

    const char *sql =
        "SELECT b.verse, b.content "
        "FROM bible b "
        "JOIN bible_books bb ON bb.abbr = b.book "
        "WHERE bb.name = ? AND b.chapter = ? "
        "ORDER BY b.verse "
        "LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        qWarning() << "Failed to prepare verses query:" << sqlite3_errmsg(db);
        return list;
    }

    sqlite3_bind_text(stmt, 1, bookName.toLocal8Bit().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, chapter);
    sqlite3_bind_int(stmt, 3, limit);
    sqlite3_bind_int(stmt, 4, offset);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int verseNum = sqlite3_column_int(stmt, 0);
        const unsigned char *content = sqlite3_column_text(stmt, 1);
        QString verseText;
        if (content) {
            verseText = QString::fromUtf8(reinterpret_cast<const char*>(content));
        } else {
            verseText = QString();
        }
        list.emplace_back(verseNum, verseText);
    }
    sqlite3_finalize(stmt);
    return list;
}
