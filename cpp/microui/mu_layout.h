#pragma once
#include "mu_container.h"
#include "mu_stack.h"
#include "mu_style.h"
#include <UIRenderFrame.h>
#include <assert.h>
#include <string.h>

const auto MU_LAYOUTSTACK_SIZE = 16;
const int MU_MAX_WIDTHS = 16;

// enum LAYOUT { LAYOUT_NONE, LAYOUT_RELATIVE = 1, LAYOUT_ABSOLUTE = 2 };

class mu_Layout {
  // friend class MuLayoutManager;

  UIRect _body;

  // res.x, y
  UIVec2 _position;
  // res.h
  int _height = 0;
  //
  // res.w
  //
  // if items == 0
  int _width = 0;
  // columns
  int _widths[MU_MAX_WIDTHS] = {0};
  // columns count
  int _items = 0;
  // current index
  int _item_index = 0;

  // used area ?
  UIVec2 _max;

  // next row.x
  int indent = 0;
  // next row.y
  int next_row = 0;

  // LAYOUT next_type = LAYOUT_NONE;
  // UIRect next;

public:
  mu_Layout() {
    int width = 0;
    row(1, &width, 0);
  }

  mu_Layout(UIRect body) {
    this->_body = body;
    this->_max = UIVec2(-0x1000000, -0x1000000);
    int width = 0;
    row(1, &width, 0);
  }

  void add_indent(int x) { indent += x; }

  void row(int items, const int *widths, int height) {
    if (widths) {
      assert(items <= MU_MAX_WIDTHS);
      memcpy(this->_widths, widths, items * sizeof(widths[0]));
    }
    this->_items = items;
    this->_position = UIVec2(this->indent, this->next_row);
    this->_height = height;
    this->_item_index = 0;
  }

  UIVec2 remain() const { return {_max.x - _body.x, _max.y - _body.y}; }
  UIVec2 body_size() const { return UIVec2{_body.w, _body.h}; }

  void inherit_column(const mu_Layout &b) {
    this->_position.x =
        mu_max(this->_position.x, b._position.x + b._body.x - this->_body.x);
    this->next_row =
        mu_max(this->next_row, b.next_row + b._body.y - this->_body.y);
    this->_max.x = mu_max(this->_max.x, b._max.x);
    this->_max.y = mu_max(this->_max.y, b._max.y);
  }

  UIRect next(const mu_Style *style) {
    // mu_Layout *layout = &_layout_stack.back();

    UIRect res;
    // if (next_type) {
    //   // handle rect set by `mu_layout_set_next`
    //   int type = next_type;
    //   next_type = LAYOUT_NONE;
    //   res = next;
    //   if (type == LAYOUT_ABSOLUTE) {
    //     return res;
    //   }
    // } else
    {
      // handle next row
      if (_item_index == _items) {
        // end of row. next row.
        row(_items, nullptr, _height);
      }

      // position
      res.x = _position.x;
      res.y = _position.y;

      // size
      res.w = _items > 0 ? _widths[_item_index] : _width;
      res.h = _height;
      if (res.w == 0) {
        res.w = style->size.x + style->padding * 2;
      }
      if (res.h == 0) {
        res.h = style->size.y + style->padding * 2;
      }
      if (res.w < 0) {
        res.w += _body.w - res.x + 1;
      }
      if (res.h < 0) {
        res.h += _body.h - res.y + 1;
      }

      _item_index++;
    }

    // update position
    _position.x += res.w + style->spacing;
    next_row = mu_max(next_row, res.y + res.h + style->spacing);

    // apply body offset
    res.x += _body.x;
    res.y += _body.y;

    // update max position
    _max.x = mu_max(_max.x, res.x + res.w);
    _max.y = mu_max(_max.y, res.y + res.h);

    return res;
  }

  // void set_width(int width) { size.x = width; }
  // void set_height(int height) { size.y = height; }
  // void set_next(UIRect r, bool relative) {
  //   // mu_Layout *layout = &_layout_stack.back();
  //   next = r;
  //   next_type = relative ? LAYOUT_RELATIVE : LAYOUT_ABSOLUTE;
  // }
};

class MuLayoutManager {
  mu_Stack<mu_Layout, MU_LAYOUTSTACK_SIZE> _layout_stack;

public:
  void end() { assert(_layout_stack.size() == 0); }

  void push(const mu_Layout &layout) { _layout_stack.push(layout); }
  mu_Layout &back() { return _layout_stack.back(); }
  mu_Layout *pop() {
    auto back = &_layout_stack.back();
    _layout_stack.pop();
    return back;
  }

  mu_Layout *begin_column(const mu_Style *style) {
    auto &back = _layout_stack.back();
    _layout_stack.push(mu_Layout(back.next(style)));
    return &_layout_stack.back();
  }

  void end_column() {
    auto &b = _layout_stack.back();
    _layout_stack.pop();
    // inherit position/next_row/max from child layout if they are greater
    auto a = &_layout_stack.back();
    a->inherit_column(b);
  }
};
