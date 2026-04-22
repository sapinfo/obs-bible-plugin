#ifndef BIBLE_PLUGIN_MANAGER_H
#define BIBLE_PLUGIN_MANAGER_H

#include <QObject>
#include <QString>
#include <vector>
#include <utility>

#include "BibleSource.h"
#include "BibleResultHandler.h"
#include "BiblePluginSettings.h"

class BiblePluginManager : public QObject {
    Q_OBJECT
    
public:
    BiblePluginManager(const BiblePluginSettings &initial_settings);
    ~BiblePluginManager();
    
    void update_settings(const BiblePluginSettings &new_settings);
    BiblePluginSettings get_settings() const;
    
    // Bible data access
    std::vector<QString> get_book_list() const;
    int get_chapter_count(const QString &book_name) const;
    std::vector<std::pair<int, QString>> get_verses(const QString &book_name, int chapter, int offset, int limit) const;
    
    // Settings access for UI
    BibleSource* get_bible_source() { return &bible_source; }
    BibleResultHandler* get_result_handler() { return &result_handler; }

private:
    BiblePluginSettings settings;
    BibleSource bible_source;
    BibleResultHandler result_handler;
};

#endif // BIBLE_PLUGIN_MANAGER_H
