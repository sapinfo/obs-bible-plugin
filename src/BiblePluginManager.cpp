#include "BiblePluginManager.h"

BiblePluginManager::BiblePluginManager(const BiblePluginSettings &initial_settings)
    : settings(initial_settings)
{
    // Load the default bible.db shipped with the plugin.
    // If it fails, UI will show empty lists; caller may later initialize explicitly.
    bible_source.initializeDefault();
}

BiblePluginManager::~BiblePluginManager()
{
    // No special cleanup needed
}

void BiblePluginManager::update_settings(const BiblePluginSettings &new_settings)
{
    settings = new_settings;
    // Optionally, notify listeners if any
}

BiblePluginSettings BiblePluginManager::get_settings() const
{
    return settings;
}

std::vector<QString> BiblePluginManager::get_book_list() const
{
    return bible_source.get_book_list();
}

int BiblePluginManager::get_chapter_count(const QString &book_name) const
{
    return bible_source.get_chapter_count(book_name);
}

std::vector<std::pair<int, QString>> BiblePluginManager::get_verses(const QString &book_name, int chapter, int offset, int limit) const
{
    return bible_source.get_verses(book_name, chapter, offset, limit);
}
