#ifndef BIBLE_PLUGIN_SETTINGS_H
#define BIBLE_PLUGIN_SETTINGS_H

#include <QString>
#include <obs-data.h>

struct BiblePluginSettings {
    bool enabled;
    int fontSize;
    QString fontFamily;
    bool autoScroll;

    BiblePluginSettings()
        : enabled(false), fontSize(12), fontFamily("Arial"), autoScroll(true) {}
};

// obs_data_t serialization helpers. Key namespace: "bible_plugin.*"
void save_BiblePluginSettings_to_config(obs_data_t *save_data, const BiblePluginSettings &settings);
BiblePluginSettings load_BiblePluginSettings_from_config(obs_data_t *save_data);

#endif // BIBLE_PLUGIN_SETTINGS_H
