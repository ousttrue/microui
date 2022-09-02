#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum { ATLAS_WHITE = 5 /*MU_ICON_MAX*/, ATLAS_FONT };
enum { ATLAS_WIDTH = 128, ATLAS_HEIGHT = 128 };
extern unsigned char atlas_texture[];

struct Rect {
  int x, y, w, h;
};
extern struct Rect atlas[];

#ifdef __cplusplus
}
#endif
