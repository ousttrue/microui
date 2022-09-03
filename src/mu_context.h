#pragma once
#include "mu_container.h"
#include "mu_input.h"
#include "mu_layout.h"
#include "mu_pool.h"
#include "mu_style.h"
#include "mu_hash.h"
#include "mu_command.h"
#include "mu_clip.h"
#include <assert.h>

#define MU_MAX_FMT 127
#define MU_LAYOUTSTACK_SIZE 16
#define MU_TREENODEPOOL_SIZE 48

using text_width_callback = int (*)(mu_Font font, const char *str, int len);
using text_height_callback = int (*)(mu_Font font);
using draw_frame_callback = void (*)(struct mu_Context *ctx, UIRect rect,
                                     int colorid);

struct mu_Context {
  mu_Context(const mu_Context &) = delete;
  mu_Context &operator=(const mu_Context &) = delete;

  void draw_box(UIRect rect, UIColor32 color) {
    this->draw_rect(UIRect(rect.x + 1, rect.y, rect.w - 2, 1), color);
    this->draw_rect(UIRect(rect.x + 1, rect.y + rect.h - 1, rect.w - 2, 1),
                    color);
    this->draw_rect(UIRect(rect.x, rect.y, 1, rect.h), color);
    this->draw_rect(UIRect(rect.x + rect.w - 1, rect.y, 1, rect.h), color);
  }

  static void default_draw_frame(mu_Context *ctx, UIRect rect, int colorid) {
    ctx->draw_rect(rect, ctx->style->colors[colorid]);
    if (colorid == MU_STYLE_SCROLLBASE || colorid == MU_STYLE_SCROLLTHUMB ||
        colorid == MU_STYLE_TITLEBG) {
      return;
    }
    // draw border
    if (ctx->style->colors[MU_STYLE_BORDER].a) {
      ctx->draw_box(rect.expand(1), ctx->style->colors[MU_STYLE_BORDER]);
    }
  }

  mu_Context() {
    this->draw_frame = default_draw_frame;
    this->_style = {};
    this->style = &this->_style;
  }

  // callbacks
  text_width_callback text_width = nullptr;
  text_height_callback text_height = nullptr;
  draw_frame_callback draw_frame = nullptr;

  // core state
  MuHash _hash;
  mu_Style _style = {};
  mu_Style *style = nullptr;
  mu_Id hover = 0;
  UIRect last_rect;
  int frame = 0;
  mu_Container *scroll_target = nullptr;
  char number_edit_buf[MU_MAX_FMT] = {0};
  mu_Id number_edit = 0;
  mu_Id focus = 0;
  bool updated_focus = false;

  // stacks
  ContainerManager _container;
  CommandStack _command_stack;
  ClipStack _clip_stack;
  mu_Stack<mu_Layout, MU_LAYOUTSTACK_SIZE> layout_stack;

  // retained state pools
  mu_Pool<MU_TREENODEPOOL_SIZE> treenode_pool;

  // input state
  mu_Input _input;

public:
  void draw_rect(UIRect rect, const UIColor32 &color) {
    rect = _clip_stack.intersect(rect);
    if (rect.w > 0 && rect.h > 0) {
      _command_stack.push_rect(rect, color);
    }
  }

  void set_focus(mu_Id id) {
    focus = id;
    updated_focus = true;
  }

  void focus_last()
  {
    set_focus(_hash.last());
  }

  void unset_focus() {
    if (!updated_focus) {
      focus = 0;
    }
    updated_focus = false;
  }

  bool has_focus(mu_Id id) const { return focus == id; }

  void update_focus(mu_Id id) {
    if (focus == id) {
      updated_focus = true;
    }
  }

  void end_input() {
    if (this->scroll_target) {
      this->scroll_target->scroll += this->_input.scroll_delta();
    }

    // unset focus if focus id was not touched this frame
    this->unset_focus();

    // reset input state
    this->_input.end();
  }
};
