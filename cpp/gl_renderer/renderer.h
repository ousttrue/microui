#pragma once

#ifdef BUILD_GLRENDERER
#define EXPORT_GLRENDERER __declspec(dllexport)
#else
#define EXPORT_GLRENDERER __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT_GLRENDERER void r_init(void);
EXPORT_GLRENDERER int r_get_text_width(const char *text, int len);
EXPORT_GLRENDERER int r_get_text_height(void);
EXPORT_GLRENDERER void render(int width, int height, float bg[4],
                              const struct UIRenderFrame *command);

#ifdef __cplusplus
}
#endif
