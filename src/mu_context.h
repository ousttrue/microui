#pragma once
#include "mu_container.h"
#include "mu_layout.h"
#include "mu_pool.h"
#include "mu_rect.h"
#include "mu_style.h"
#include "mu_input.h"
#include <assert.h>

#define MU_ROOTLIST_SIZE 32
#define MU_CONTAINERSTACK_SIZE 32
#define MU_CLIPSTACK_SIZE 32
#define MU_IDSTACK_SIZE 32
#define MU_LAYOUTSTACK_SIZE 16
#define MU_CONTAINERPOOL_SIZE 48
#define MU_TREENODEPOOL_SIZE 48

enum class MU_CLIP : unsigned int { NONE, PART, ALL };

using text_width_callback = int (*)(mu_Font font, const char *str, int len);
using text_height_callback = int (*)(mu_Font font);
using draw_frame_callback = void (*)(struct mu_Context *ctx, mu_Rect rect,
                                     int colorid);

struct mu_Context {
  mu_Context(const mu_Context &) = delete;
  mu_Context &operator=(const mu_Context &) = delete;

  void draw_box(mu_Rect rect, mu_Color color) {
    this->draw_rect(mu_Rect(rect.x + 1, rect.y, rect.w - 2, 1), color);
    this->draw_rect(mu_Rect(rect.x + 1, rect.y + rect.h - 1, rect.w - 2, 1),
                    color);
    this->draw_rect(mu_Rect(rect.x, rect.y, 1, rect.h), color);
    this->draw_rect(mu_Rect(rect.x + rect.w - 1, rect.y, 1, rect.h), color);
  }

  static void default_draw_frame(mu_Context *ctx, mu_Rect rect, int colorid) {
    ctx->draw_rect(rect, ctx->style->colors[colorid]);
    if (colorid == MU_COLOR_SCROLLBASE || colorid == MU_COLOR_SCROLLTHUMB ||
        colorid == MU_COLOR_TITLEBG) {
      return;
    }
    // draw border
    if (ctx->style->colors[MU_COLOR_BORDER].a) {
      ctx->draw_box(rect.expand(1), ctx->style->colors[MU_COLOR_BORDER]);
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
  mu_Style _style = {};
  mu_Style *style = nullptr;
  mu_Id hover = 0;
  mu_Id last_id = 0;
  mu_Rect last_rect;
  int last_zindex = 0;
  int frame = 0;
  mu_Container *hover_root = nullptr;
  mu_Container *next_hover_root = nullptr;
  mu_Container *scroll_target = nullptr;
  char number_edit_buf[MU_MAX_FMT] = {0};
  mu_Id number_edit = 0;

  // stacks
  CommandStack _command_stack;
  mu_Stack<mu_Container *, MU_ROOTLIST_SIZE> root_list;
  mu_Stack<mu_Container *, MU_CONTAINERSTACK_SIZE> container_stack;
  mu_Stack<mu_Rect, MU_CLIPSTACK_SIZE> clip_stack;

  mu_Stack<mu_Id, MU_IDSTACK_SIZE> id_stack;
  mu_Stack<mu_Layout, MU_LAYOUTSTACK_SIZE> layout_stack;

  // retained state pools
  mu_Pool<MU_CONTAINERPOOL_SIZE> container_pool;
  mu_Container containers[MU_CONTAINERPOOL_SIZE] = {0};
  mu_Pool<MU_TREENODEPOOL_SIZE> treenode_pool;

  // input state
  mu_Input _input;

private:
  mu_Id focus = 0;
  bool updated_focus = false;

public:
  void draw_rect(mu_Rect rect, mu_Color color) {
    rect = rect.intersect(this->clip_stack.back());
    if (rect.w > 0 && rect.h > 0) {
      _command_stack.push_rect(rect, color);
    }
  }

  void push_clip_rect(mu_Rect rect) {
    mu_Rect last = this->clip_stack.back();
    this->clip_stack.push(rect.intersect(last));
  }

  void pop_clip_rect() { this->clip_stack.pop(); }

  MU_CLIP check_clip(mu_Rect r) {
    mu_Rect cr = this->clip_stack.back();
    if (r.x > cr.x + cr.w || r.x + r.w < cr.x || r.y > cr.y + cr.h ||
        r.y + r.h < cr.y) {
      return MU_CLIP::ALL;
    }
    if (r.x >= cr.x && r.x + r.w <= cr.x + cr.w && r.y >= cr.y &&
        r.y + r.h <= cr.y + cr.h) {
      return MU_CLIP::NONE;
    }
    return MU_CLIP::PART;
  }

  void set_focus(mu_Id id) {
    focus = id;
    updated_focus = 1;
  }

  void unset_focus() {
    if (!updated_focus) {
      focus = 0;
    }
    updated_focus = 0;
  }

  bool has_focus(mu_Id id) const { return focus == id; }

  void update_focus(mu_Id id) {
    if (focus == id) {
      updated_focus = true;
    }
  }

  void bring_to_front(mu_Container *cnt) { cnt->zindex = ++this->last_zindex; }

  void end_input() {
    if (this->scroll_target) {
      this->scroll_target->scroll += this->_input.scroll_delta();
    }

    // unset focus if focus id was not touched this frame
    this->unset_focus();

    // bring hover root to front if mouse was pressed
    if (this->_input.mouse_pressed() && this->next_hover_root &&
        this->next_hover_root->zindex < this->last_zindex &&
        this->next_hover_root->zindex >= 0) {
      this->bring_to_front(this->next_hover_root);
    }

    // reset input state
    this->_input.end();
  }
};
