#pragma once
#include "mu_container.h"
#include "mu_stack.h"
#include "mu_style.h"
#include <UIRenderFrame.h>
#include <assert.h>
#include <string.h>

const auto MU_LAYOUTSTACK_SIZE = 16;
const int MU_MAX_WIDTHS = 16;

enum LAYOUT { LAYOUT_NONE, LAYOUT_RELATIVE = 1, LAYOUT_ABSOLUTE = 2 };

class mu_Layout {
  friend class MuLayoutManager;

  UIRect body;
  UIRect next;
  UIVec2 position;
  UIVec2 size;
  UIVec2 max;
  int widths[MU_MAX_WIDTHS] = {0};
  int items = 0;
  int item_index = 0;
  int next_row = 0;
  LAYOUT next_type = LAYOUT_NONE;
  int indent = 0;

public:
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

  void add_indent(int x) { indent += x; }
  void resize_container(mu_Container *cnt) const {
    cnt->rect.w = cnt->content_size.x + (cnt->rect.w - body.w);
    cnt->rect.h = cnt->content_size.y + (cnt->rect.h - body.h);
  }
  void fit(mu_Container *cnt) const {
    cnt->content_size.x = max.x - body.x;
    cnt->content_size.y = max.y - body.y;
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

class MuLayoutManager {
  mu_Stack<mu_Layout, MU_LAYOUTSTACK_SIZE> _layout_stack;
  UIRect _last_rect = {};

public:
  void end() { assert(_layout_stack.size() == 0); }

  void push(const mu_Layout &layout) { _layout_stack.push(layout); }
  mu_Layout &back() { return _layout_stack.back(); }
  mu_Layout *pop() {
    auto back = &_layout_stack.back();
    _layout_stack.pop();
    return back;
  }

  void begin_column(const mu_Style *style) {
    _layout_stack.push(mu_Layout(next(style), UIVec2(0, 0)));
  }

  void end_column() {
    auto b = &_layout_stack.back();
    _layout_stack.pop();
    // inherit position/next_row/max from child layout if they are greater
    auto a = &_layout_stack.back();
    a->position.x =
        mu_max(a->position.x, b->position.x + b->body.x - a->body.x);
    a->next_row = mu_max(a->next_row, b->next_row + b->body.y - a->body.y);
    a->max.x = mu_max(a->max.x, b->max.x);
    a->max.y = mu_max(a->max.y, b->max.y);
  }

  void set_width(int width) { _layout_stack.back().size.x = width; }

  void set_height(int height) { _layout_stack.back().size.y = height; }

  void set_next(UIRect r, bool relative) {
    mu_Layout *layout = &_layout_stack.back();
    layout->next = r;
    layout->next_type = relative ? LAYOUT_RELATIVE : LAYOUT_ABSOLUTE;
  }

  UIRect next(const mu_Style *style) {
    // auto style = _command_drawer.style();
    mu_Layout *layout = &_layout_stack.back();

    UIRect res;
    if (layout->next_type) {
      // handle rect set by `mu_layout_set_next`
      int type = layout->next_type;
      layout->next_type = LAYOUT_NONE;
      res = layout->next;
      if (type == LAYOUT_ABSOLUTE) {
        return (_last_rect = res);
      }
    } else {
      // handle next row
      if (layout->item_index == layout->items) {
        _layout_stack.back().row(layout->items, nullptr, layout->size.y);
      }

      // position
      res.x = layout->position.x;
      res.y = layout->position.y;

      // size
      res.w = layout->items > 0 ? layout->widths[layout->item_index]
                                : layout->size.x;
      res.h = layout->size.y;
      if (res.w == 0) {
        res.w = style->size.x + style->padding * 2;
      }
      if (res.h == 0) {
        res.h = style->size.y + style->padding * 2;
      }
      if (res.w < 0) {
        res.w += layout->body.w - res.x + 1;
      }
      if (res.h < 0) {
        res.h += layout->body.h - res.y + 1;
      }

      layout->item_index++;
    }

    // update position
    layout->position.x += res.w + style->spacing;
    layout->next_row = mu_max(layout->next_row, res.y + res.h + style->spacing);

    // apply body offset
    res.x += layout->body.x;
    res.y += layout->body.y;

    // update max position
    layout->max.x = mu_max(layout->max.x, res.x + res.w);
    layout->max.y = mu_max(layout->max.y, res.y + res.h);

    return (_last_rect = res);
  }
};
