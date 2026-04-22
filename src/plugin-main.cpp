#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/base.h>

#include <QMainWindow>
#include <QMetaObject>

#include "BiblePluginManager.h"
#include "BiblePluginSettings.h"
#include "BibleVerseSource.h"
#include "ui/BibleDock.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-bible-plugin", "en-US")

static BibleDock *g_dock = nullptr;

static obs_hotkey_id g_hk_next   = OBS_INVALID_HOTKEY_ID;
static obs_hotkey_id g_hk_prev   = OBS_INVALID_HOTKEY_ID;
static obs_hotkey_id g_hk_toggle = OBS_INVALID_HOTKEY_ID;

static void hk_cb(void *, obs_hotkey_id id, obs_hotkey_t *, bool pressed) {
    if (!pressed) return;
    auto *mgr = BiblePluginManager::instance();
    if (id == g_hk_next) {
        QMetaObject::invokeMethod(mgr, "step", Qt::QueuedConnection, Q_ARG(int, +1));
    } else if (id == g_hk_prev) {
        QMetaObject::invokeMethod(mgr, "step", Qt::QueuedConnection, Q_ARG(int, -1));
    } else if (id == g_hk_toggle) {
        QMetaObject::invokeMethod(mgr, "set_visible", Qt::QueuedConnection,
                                  Q_ARG(bool, !mgr->visible()));
    }
}

static void register_hotkeys() {
    g_hk_next   = obs_hotkey_register_frontend("bible.next",
        obs_module_text("BibleHotkeyNext"), hk_cb, nullptr);
    g_hk_prev   = obs_hotkey_register_frontend("bible.prev",
        obs_module_text("BibleHotkeyPrev"), hk_cb, nullptr);
    g_hk_toggle = obs_hotkey_register_frontend("bible.toggle",
        obs_module_text("BibleHotkeyToggle"), hk_cb, nullptr);
}

static void unregister_hotkeys() {
    if (g_hk_next   != OBS_INVALID_HOTKEY_ID) obs_hotkey_unregister(g_hk_next);
    if (g_hk_prev   != OBS_INVALID_HOTKEY_ID) obs_hotkey_unregister(g_hk_prev);
    if (g_hk_toggle != OBS_INVALID_HOTKEY_ID) obs_hotkey_unregister(g_hk_toggle);
    g_hk_next = g_hk_prev = g_hk_toggle = OBS_INVALID_HOTKEY_ID;
}

static void setup_dock() {
    if (g_dock) return;
    g_dock = new BibleDock();
    obs_frontend_add_dock_by_id("bible_dock", obs_module_text("BibleDock"), g_dock);
}

static void on_frontend_event(enum obs_frontend_event ev, void *) {
    if (ev == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
        BiblePluginManager::instance()->initialize();
        setup_dock();
    } else if (ev == OBS_FRONTEND_EVENT_EXIT) {
        g_dock = nullptr;
    }
}

static void on_save_or_load(obs_data_t *save_data, bool saving, void *) {
    auto *mgr = BiblePluginManager::instance();
    if (saving) {
        save_BiblePluginSettings_to_config(save_data, mgr->export_settings());
    } else {
        BiblePluginSettings s = load_BiblePluginSettings_from_config(save_data);
        mgr->import_settings(s);
    }
}

MODULE_EXPORT const char *obs_module_description(void) {
    return "Displays Bible verses in OBS";
}
MODULE_EXPORT const char *obs_module_name(void) {
    return "Bible";
}

bool obs_module_load(void) {
    blog(LOG_INFO, "[obs-bible-plugin] module_load");
    bible_verse_source_register();
    register_hotkeys();
    obs_frontend_add_event_callback(on_frontend_event, nullptr);
    obs_frontend_add_save_callback(on_save_or_load, nullptr);
    return true;
}

void obs_module_unload(void) {
    blog(LOG_INFO, "[obs-bible-plugin] module_unload");
    obs_frontend_remove_save_callback(on_save_or_load, nullptr);
    obs_frontend_remove_event_callback(on_frontend_event, nullptr);
    unregister_hotkeys();
    BiblePluginManager::instance()->shutdown();
}
