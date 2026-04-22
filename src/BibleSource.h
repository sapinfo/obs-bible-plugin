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
