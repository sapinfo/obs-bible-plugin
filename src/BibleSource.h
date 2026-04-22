#ifndef BIBLE_SOURCE_H
#define BIBLE_SOURCE_H

#include <QString>
#include <vector>
#include <utility>
#include <sqlite3.h>

class BibleSource {
public:
    BibleSource();
    ~BibleSource();

    // Initialize with the path to the bible.db file
    bool initialize(const QString &dbPath);

    // Initialize with the default DB path shipped with the plugin.
    // Uses: <plugin root>/data/bible.db
    bool initializeDefault();

    // Get list of book names (Korean, sorted alphabetically)
    std::vector<QString> get_book_list() const;

    // Get the number of chapters for a given book (by Korean name)
    int get_chapter_count(const QString &bookName) const;

    // Get verses for a given book and chapter, with pagination
    // Returns a vector of pairs (verse number, content)
    std::vector<std::pair<int, QString>> get_verses(const QString &bookName, int chapter, int offset, int limit) const;

private:
    sqlite3 *db = nullptr;
    QString dbPath;
};

#endif // BIBLE_SOURCE_H
