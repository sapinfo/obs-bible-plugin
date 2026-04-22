#ifndef BIBLE_PLUGIN_SETTINGS_H
#define BIBLE_PLUGIN_SETTINGS_H

#include <QString>
#include <obs-data.h>

struct BiblePluginSettings {
    QString last_query;
    int     last_index   = -1;
    bool    last_visible = true;
};

void save_BiblePluginSettings_to_config(obs_data_t *save_data, const BiblePluginSettings &s);
BiblePluginSettings load_BiblePluginSettings_from_config(obs_data_t *save_data);

#endif
