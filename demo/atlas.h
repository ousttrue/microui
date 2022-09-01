#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <microui.h>
enum { ATLAS_WHITE = MU_ICON_MAX, ATLAS_FONT };
enum { ATLAS_WIDTH = 128, ATLAS_HEIGHT = 128 };
extern unsigned char atlas_texture[ATLAS_WIDTH * ATLAS_HEIGHT];
extern mu_Rect atlas[];

#ifdef __cplusplus
}
#endif