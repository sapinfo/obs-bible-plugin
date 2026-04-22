#ifndef BIBLE_PLUGIN_MANAGER_H
#define BIBLE_PLUGIN_MANAGER_H

#include <QObject>
#include <QString>
#include <vector>

#include "BibleSource.h"
#include "BiblePluginSettings.h"

class BiblePluginManager : public QObject {
    Q_OBJECT
public:
    static BiblePluginManager *instance();

    void initialize();
    void shutdown();

    Q_INVOKABLE void apply_search(const QString &query);
    Q_INVOKABLE void select_index(int i);
    Q_INVOKABLE void step(int delta);
    Q_INVOKABLE void set_visible(bool v);

    const std::vector<VerseRow> &filtered_list() const { return filtered; }
    std::vector<BookRow> get_books() const { return bible_source.get_books(); }
    int  current_index() const { return cur_index; }
    bool visible()       const { return is_visible; }
    QString current_query() const { return last_query; }

    void import_settings(const BiblePluginSettings &s);
    BiblePluginSettings export_settings() const;

signals:
    void list_changed();
    void verse_changed(QString reference, QString body, bool visible);

private:
    BiblePluginManager();
    ~BiblePluginManager() override;
    void emit_current_verse();

    BibleSource bible_source;
    std::vector<VerseRow> filtered;
    QString last_query;
    int     cur_index = -1;
    bool    is_visible = true;
    bool    initialized = false;
};

#endif
