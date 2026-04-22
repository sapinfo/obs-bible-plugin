#include "BibleVerseSource.h"
#include "BiblePluginManager.h"

#include <obs-module.h>
#include <util/base.h>
#include <QMetaObject>
#include <QString>
#include <string>

namespace {

struct bible_verse_source {
    obs_source_t *src = nullptr;
    obs_source_t *text_source = nullptr;
    QMetaObject::Connection verse_conn;
};

static void apply_text(obs_source_t *ts, const QString &line) {
    if (!ts) return;
    obs_data_t *d = obs_data_create();
    obs_data_set_string(d, "text", line.toUtf8().constData());
    obs_source_update(ts, d);
    obs_data_release(d);
}

static QString make_display(const QString &ref, const QString &body, bool visible) {
    if (!visible) return {};
    if (ref.isEmpty() && body.isEmpty()) return {};
    if (ref.isEmpty()) return body;
    if (body.isEmpty()) return ref;
    return ref + "  " + body;
}

static obs_source_t *create_text_source(obs_data_t *src_settings) {
    obs_data_t *d = obs_data_create();
    obs_data_t *font = obs_data_get_obj(src_settings, "font");
    if (!font) {
        font = obs_data_create();
        obs_data_set_string(font, "face",  "Apple SD Gothic Neo");
        obs_data_set_string(font, "style", "Regular");
        obs_data_set_int   (font, "size",  48);
        obs_data_set_int   (font, "flags", 0);
    }
    obs_data_set_obj(d, "font", font);
    obs_data_release(font);

    obs_data_set_int (d, "color1", obs_data_get_int(src_settings, "color1"));
    obs_data_set_int (d, "color2", obs_data_get_int(src_settings, "color1"));
    obs_data_set_bool(d, "outline", obs_data_get_bool(src_settings, "outline"));
    obs_data_set_bool(d, "drop_shadow", obs_data_get_bool(src_settings, "drop_shadow"));
    obs_data_set_bool(d, "word_wrap", obs_data_get_bool(src_settings, "word_wrap"));
    obs_data_set_int (d, "custom_width", obs_data_get_int(src_settings, "custom_width"));
    obs_data_set_string(d, "text", "");

    obs_source_t *ts = obs_source_create_private(
        "text_ft2_source_v2", "obs-bible-plugin.text_source", d);
    obs_data_release(d);
    return ts;
}

static const char *bvs_get_name(void *) {
    return obs_module_text("BibleVerseSource");
}

static void *bvs_create(obs_data_t *settings, obs_source_t *source) {
    auto *ctx = new bible_verse_source();
    ctx->src = source;
    ctx->text_source = create_text_source(settings);

    auto *mgr = BiblePluginManager::instance();
    ctx->verse_conn = QObject::connect(
        mgr, &BiblePluginManager::verse_changed,
        mgr,  // receiver (must be a QObject; disconnect is via Connection)
        [ctx](QString ref, QString body, bool visible) {
            apply_text(ctx->text_source, make_display(ref, body, visible));
        },
        Qt::QueuedConnection);

    // Initial push: reflect current manager state once.
    int i = mgr->current_index();
    const auto &lst = mgr->filtered_list();
    if (i >= 0 && i < (int)lst.size()) {
        const VerseRow &r = lst[i];
        apply_text(ctx->text_source, make_display(r.verse_ref, r.content, mgr->visible()));
    } else {
        apply_text(ctx->text_source, {});
    }
    return ctx;
}

static void bvs_destroy(void *data) {
    auto *ctx = static_cast<bible_verse_source *>(data);
    if (!ctx) return;
    QObject::disconnect(ctx->verse_conn);
    if (ctx->text_source) obs_source_release(ctx->text_source);
    delete ctx;
}

static uint32_t bvs_get_width(void *data) {
    auto *ctx = static_cast<bible_verse_source *>(data);
    return (ctx && ctx->text_source) ? obs_source_get_width(ctx->text_source) : 0;
}
static uint32_t bvs_get_height(void *data) {
    auto *ctx = static_cast<bible_verse_source *>(data);
    return (ctx && ctx->text_source) ? obs_source_get_height(ctx->text_source) : 0;
}
static void bvs_video_render(void *data, gs_effect_t *) {
    auto *ctx = static_cast<bible_verse_source *>(data);
    if (ctx && ctx->text_source) obs_source_video_render(ctx->text_source);
}

static obs_properties_t *bvs_get_properties(void *) {
    obs_properties_t *p = obs_properties_create();
    obs_properties_add_font(p, "font", "Font");
    obs_properties_add_color(p, "color1", "Color");
    obs_properties_add_bool (p, "outline", "Outline");
    obs_properties_add_bool (p, "drop_shadow", "Drop Shadow");
    obs_properties_add_bool (p, "word_wrap", "Word Wrap");
    obs_properties_add_int  (p, "custom_width", "Custom Width", 0, 8192, 1);
    return p;
}

static void bvs_get_defaults(obs_data_t *d) {
    obs_data_t *font = obs_data_create();
    obs_data_set_default_string(font, "face",  "Apple SD Gothic Neo");
    obs_data_set_default_string(font, "style", "Regular");
    obs_data_set_default_int   (font, "size",  48);
    obs_data_set_default_int   (font, "flags", 0);
    obs_data_set_default_obj(d, "font", font);
    obs_data_release(font);
    obs_data_set_default_int (d, "color1", 0xFFFFFFFF);
    obs_data_set_default_bool(d, "outline", false);
    obs_data_set_default_bool(d, "drop_shadow", false);
    obs_data_set_default_bool(d, "word_wrap", false);
    obs_data_set_default_int (d, "custom_width", 0);
}

static void bvs_update(void *data, obs_data_t *settings) {
    auto *ctx = static_cast<bible_verse_source *>(data);
    if (!ctx || !ctx->text_source) return;

    obs_data_t *cur = obs_source_get_settings(ctx->text_source);
    std::string preserve = obs_data_get_string(cur, "text");
    obs_data_release(cur);

    obs_data_t *d = obs_data_create();
    obs_data_t *font = obs_data_get_obj(settings, "font");
    if (font) {
        obs_data_set_obj(d, "font", font);
        obs_data_release(font);
    }
    obs_data_set_int (d, "color1", obs_data_get_int(settings, "color1"));
    obs_data_set_int (d, "color2", obs_data_get_int(settings, "color1"));
    obs_data_set_bool(d, "outline", obs_data_get_bool(settings, "outline"));
    obs_data_set_bool(d, "drop_shadow", obs_data_get_bool(settings, "drop_shadow"));
    obs_data_set_bool(d, "word_wrap", obs_data_get_bool(settings, "word_wrap"));
    obs_data_set_int (d, "custom_width", obs_data_get_int(settings, "custom_width"));
    obs_data_set_string(d, "text", preserve.c_str());
    obs_source_update(ctx->text_source, d);
    obs_data_release(d);
}

static struct obs_source_info g_info = {};

} // anonymous namespace

void bible_verse_source_register() {
    g_info.id             = "obs-bible-plugin.bible_verse";
    g_info.type           = OBS_SOURCE_TYPE_INPUT;
    g_info.output_flags   = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
    g_info.icon_type      = OBS_ICON_TYPE_TEXT;
    g_info.get_name       = bvs_get_name;
    g_info.create         = bvs_create;
    g_info.destroy        = bvs_destroy;
    g_info.get_width      = bvs_get_width;
    g_info.get_height     = bvs_get_height;
    g_info.video_render   = bvs_video_render;
    g_info.get_properties = bvs_get_properties;
    g_info.get_defaults   = bvs_get_defaults;
    g_info.update         = bvs_update;
    obs_register_source(&g_info);
}
