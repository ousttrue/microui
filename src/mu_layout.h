#pragma once
#include "mu_rect.h"
#include "mu_vec2.h"
const int MU_MAX_WIDTHS = 16;

struct mu_Layout {
  mu_Rect body;
  mu_Rect next;
  mu_Vec2 position;
  mu_Vec2 size;
  mu_Vec2 max;
  int widths[MU_MAX_WIDTHS];
  int items;
  int item_index;
  int next_row;
  int next_type;
  int indent;
};
