#pragma once
#include <mu_render_command.h>

#ifdef BUILD_MICROUI
#define EXPORT_MICROUI __declspec(dllexport)
#else
#define EXPORT_MICROUI __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT_MICROUI void r_init(void);
EXPORT_MICROUI void r_resize(int w, int h);
EXPORT_MICROUI void r_draw_rect(mu_Rect rect, mu_Color color);
EXPORT_MICROUI void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color);
EXPORT_MICROUI void r_draw_icon(int id, mu_Rect rect, mu_Color color);
EXPORT_MICROUI int r_get_text_width(const char *text, int len);
EXPORT_MICROUI int r_get_text_height(void);
EXPORT_MICROUI void r_set_clip_rect(mu_Rect rect);
EXPORT_MICROUI void r_clear(mu_Color color);
EXPORT_MICROUI void flush();
EXPORT_MICROUI void render(int width, int height, float bg[4], const mu_RenderCommand *command);

#ifdef __cplusplus
}
#endif
