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
  int widths[MU_MAX_WIDTHS] = {0};
  int items = 0;
  int item_index = 0;
  int next_row = 0;
  int next_type = 0;
  int indent = 0;

  mu_Layout() {
    int width = 0;
    row(1, &width, 0);
  }

  mu_Layout(mu_Rect body, mu_Vec2 scroll) {
    this->body = mu_Rect(body.x - scroll.x, body.y - scroll.y, body.w, body.h);
    this->max = mu_Vec2(-0x1000000, -0x1000000);
    int width = 0;
    row(1, &width, 0);
  }

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
