#include "BiblePluginSettings.h"

#include <obs-data.h>

static const char *KEY_SUBOBJECT   = "bible_plugin";
static const char *KEY_ENABLED     = "enabled";
static const char *KEY_FONT_SIZE   = "font_size";
static const char *KEY_FONT_FAMILY = "font_family";
static const char *KEY_AUTO_SCROLL = "auto_scroll";

void save_BiblePluginSettings_to_config(obs_data_t *save_data, const BiblePluginSettings &settings)
{
    if (!save_data)
        return;

    obs_data_t *obj = obs_data_create();
    obs_data_set_bool(obj, KEY_ENABLED, settings.enabled);
    obs_data_set_int(obj, KEY_FONT_SIZE, settings.fontSize);
    obs_data_set_string(obj, KEY_FONT_FAMILY, settings.fontFamily.toUtf8().constData());
    obs_data_set_bool(obj, KEY_AUTO_SCROLL, settings.autoScroll);

    obs_data_set_obj(save_data, KEY_SUBOBJECT, obj);
    obs_data_release(obj);
}

BiblePluginSettings load_BiblePluginSettings_from_config(obs_data_t *save_data)
{
    BiblePluginSettings settings; // defaults

    if (!save_data)
        return settings;

    obs_data_t *obj = obs_data_get_obj(save_data, KEY_SUBOBJECT);
    if (!obj)
        return settings;

    // Seed defaults into the obj so missing keys fall back to current defaults.
    obs_data_set_default_bool(obj, KEY_ENABLED, settings.enabled);
    obs_data_set_default_int(obj, KEY_FONT_SIZE, settings.fontSize);
    obs_data_set_default_string(obj, KEY_FONT_FAMILY, settings.fontFamily.toUtf8().constData());
    obs_data_set_default_bool(obj, KEY_AUTO_SCROLL, settings.autoScroll);

    settings.enabled    = obs_data_get_bool(obj, KEY_ENABLED);
    settings.fontSize   = static_cast<int>(obs_data_get_int(obj, KEY_FONT_SIZE));
    settings.fontFamily = QString::fromUtf8(obs_data_get_string(obj, KEY_FONT_FAMILY));
    settings.autoScroll = obs_data_get_bool(obj, KEY_AUTO_SCROLL);

    obs_data_release(obj);
    return settings;
}
