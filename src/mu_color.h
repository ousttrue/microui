#pragma once

enum {
  MU_COLOR_TEXT,
  MU_COLOR_BORDER,
  MU_COLOR_WINDOWBG,
  MU_COLOR_TITLEBG,
  MU_COLOR_TITLETEXT,
  MU_COLOR_PANELBG,
  MU_COLOR_BUTTON,
  MU_COLOR_BUTTONHOVER,
  MU_COLOR_BUTTONFOCUS,
  MU_COLOR_BASE,
  MU_COLOR_BASEHOVER,
  MU_COLOR_BASEFOCUS,
  MU_COLOR_SCROLLBASE,
  MU_COLOR_SCROLLTHUMB,
  MU_COLOR_MAX
};

struct mu_Color {
  unsigned char r, g, b, a;
  mu_Color() : r(0), g(0), b(0), a(0) {}
  mu_Color(int r, int g, int b, int a) : r(r), g(g), b(b), a(a) {}
};
