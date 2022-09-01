#pragma once
#include "mu_rect.h"
#include "mu_vec2.h"
#include <assert.h>
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

  void row(int items, const int *widths, int height) {
    if (widths) {
      assert(items <= MU_MAX_WIDTHS);
      memcpy(this->widths, widths, items * sizeof(widths[0]));
    }
    this->items = items;
    this->position = mu_Vec2(this->indent, this->next_row);
    this->size.y = height;
    this->item_index = 0;
  }
};
