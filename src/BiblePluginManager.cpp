#include "BiblePluginManager.h"
#include "BibleSearch.h"
#include <util/base.h>

static BiblePluginManager *g_instance = nullptr;

BiblePluginManager *BiblePluginManager::instance() {
    if (!g_instance) g_instance = new BiblePluginManager();
    return g_instance;
}

BiblePluginManager::BiblePluginManager() = default;
BiblePluginManager::~BiblePluginManager() = default;

void BiblePluginManager::initialize() {
    if (initialized) return;
    if (!bible_source.initializeDefault()) {
        blog(LOG_ERROR, "[obs-bible-plugin] BibleSource initializeDefault failed");
    }
    initialized = true;
}

void BiblePluginManager::shutdown() {}

void BiblePluginManager::apply_search(const QString &query) {
    last_query = query;
    filtered.clear();
    cur_index = -1;

    QString pattern = make_search_pattern(query);
    if (!pattern.isEmpty()) {
        filtered = bible_source.search(pattern);
    }

    emit list_changed();
    emit verse_changed(QString(), QString(), is_visible);
}

void BiblePluginManager::select_index(int i) {
    if (i < 0 || i >= (int)filtered.size()) return;
    cur_index = i;
    emit_current_verse();
}

void BiblePluginManager::step(int delta) {
    if (filtered.empty()) return;
    int next;
    if (cur_index < 0) {
        next = (delta > 0) ? 0 : (int)filtered.size() - 1;
    } else {
        next = cur_index + delta;
        if (next < 0 || next >= (int)filtered.size()) return;
    }
    cur_index = next;
    emit_current_verse();
}

void BiblePluginManager::set_visible(bool v) {
    if (is_visible == v) return;
    is_visible = v;
    emit_current_verse();
}

void BiblePluginManager::emit_current_verse() {
    if (cur_index < 0 || cur_index >= (int)filtered.size()) {
        emit verse_changed(QString(), QString(), is_visible);
        return;
    }
    const VerseRow &r = filtered[cur_index];
    emit verse_changed(r.verse_ref, r.content, is_visible);
}

void BiblePluginManager::import_settings(const BiblePluginSettings &s) {
    is_visible = s.last_visible;
    if (!s.last_query.isEmpty()) {
        apply_search(s.last_query);
        if (s.last_index >= 0 && s.last_index < (int)filtered.size())
            select_index(s.last_index);
    }
}

BiblePluginSettings BiblePluginManager::export_settings() const {
    BiblePluginSettings s;
    s.last_query   = last_query;
    s.last_index   = cur_index;
    s.last_visible = is_visible;
    return s;
}
