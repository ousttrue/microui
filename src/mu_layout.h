#pragma once
#include <UIRenderFrame.h>
#include <assert.h>
#include <string.h>
const int MU_MAX_WIDTHS = 16;

struct mu_Layout {
  UIRect body;
  UIRect next;
  UIVec2 position;
  UIVec2 size;
  UIVec2 max;
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

  mu_Layout(UIRect body, UIVec2 scroll) {
    this->body = UIRect(body.x - scroll.x, body.y - scroll.y, body.w, body.h);
    this->max = UIVec2(-0x1000000, -0x1000000);
    int width = 0;
    row(1, &width, 0);
  }

  void row(int items, const int *widths, int height) {
    if (widths) {
      assert(items <= MU_MAX_WIDTHS);
      memcpy(this->widths, widths, items * sizeof(widths[0]));
    }
    this->items = items;
    this->position = UIVec2(this->indent, this->next_row);
    this->size.y = height;
    this->item_index = 0;
  }
};
