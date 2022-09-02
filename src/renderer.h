#pragma once
#include <UIRenderFrame.h>

#ifdef BUILD_MICROUI
#define EXPORT_MICROUI __declspec(dllexport)
#else
#define EXPORT_MICROUI __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT_MICROUI void r_init(void);
EXPORT_MICROUI int r_get_text_width(const char *text, int len);
EXPORT_MICROUI int r_get_text_height(void);
EXPORT_MICROUI void render(int width, int height, float bg[4], const struct UIRenderFrame *command);

#ifdef __cplusplus
}
#endif
