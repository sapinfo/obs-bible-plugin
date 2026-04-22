#include "BiblePluginSettings.h"

static const char *KEY_OBJ          = "bible_plugin";
static const char *KEY_LAST_QUERY   = "last_query";
static const char *KEY_LAST_INDEX   = "last_index";
static const char *KEY_LAST_VISIBLE = "last_visible";

void save_BiblePluginSettings_to_config(obs_data_t *save_data, const BiblePluginSettings &s) {
    if (!save_data) return;
    obs_data_t *obj = obs_data_create();
    obs_data_set_string(obj, KEY_LAST_QUERY, s.last_query.toUtf8().constData());
    obs_data_set_int   (obj, KEY_LAST_INDEX, s.last_index);
    obs_data_set_bool  (obj, KEY_LAST_VISIBLE, s.last_visible);
    obs_data_set_obj(save_data, KEY_OBJ, obj);
    obs_data_release(obj);
}

BiblePluginSettings load_BiblePluginSettings_from_config(obs_data_t *save_data) {
    BiblePluginSettings s;
    if (!save_data) return s;
    obs_data_t *obj = obs_data_get_obj(save_data, KEY_OBJ);
    if (!obj) return s;
    obs_data_set_default_string(obj, KEY_LAST_QUERY, "");
    obs_data_set_default_int   (obj, KEY_LAST_INDEX, -1);
    obs_data_set_default_bool  (obj, KEY_LAST_VISIBLE, true);
    s.last_query   = QString::fromUtf8(obs_data_get_string(obj, KEY_LAST_QUERY));
    s.last_index   = (int) obs_data_get_int(obj, KEY_LAST_INDEX);
    s.last_visible = obs_data_get_bool(obj, KEY_LAST_VISIBLE);
    obs_data_release(obj);
    return s;
}
