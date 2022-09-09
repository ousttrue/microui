#pragma once

#ifdef BUILD_GLRENDERER
#define EXPORT_GLRENDERER __declspec(dllexport)
#else
#define EXPORT_GLRENDERER __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT_GLRENDERER void MUI_RENDERER_init(const void *p);
EXPORT_GLRENDERER int MUI_RENDERER_get_text_width(const char *text, int len);
EXPORT_GLRENDERER int MUI_RENDERER_get_text_height(void);
EXPORT_GLRENDERER void MUI_RENDERER_render(int width, int height, float bg[4],
                                           const struct UIRenderFrame *command);

#ifdef __cplusplus
}
#endif
