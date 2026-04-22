#include <obs-module.h>
#include <util/base.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-bible-plugin", "en-US")

MODULE_EXPORT const char *obs_module_description(void) {
    return "Displays Bible verses in OBS";
}
MODULE_EXPORT const char *obs_module_name(void) {
    return "Bible";
}

bool obs_module_load(void) {
    blog(LOG_INFO, "[obs-bible-plugin] module_load");
    return true;
}
void obs_module_unload(void) {
    blog(LOG_INFO, "[obs-bible-plugin] module_unload");
}
