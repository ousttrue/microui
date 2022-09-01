#pragma once
#include "mu_color.h"
#include "mu_vec2.h"

struct mu_Style {
  mu_Font font;
  mu_Vec2 size;
  int padding;
  int spacing;
  int indent;
  int title_height;
  int scrollbar_size;
  int thumb_size;
  mu_Color colors[MU_COLOR_MAX];
};
