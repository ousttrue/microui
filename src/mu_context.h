#pragma once
#include "mu_container.h"
#include "mu_layout.h"
#include "mu_rect.h"
#include "mu_style.h"
#include <assert.h>

template <typename T, size_t N> class mu_Stack {
public:
  T items[N];
  int idx = 0;
  const T *data() const { return &items[0]; }
  T *data() { return &items[0]; }
  int size() const { return idx; }
  const T &get(int i) const { return items[i]; }
  T &get(int i) { return items[i]; }
  T &back() { return items[idx - 1]; }
  T *next() { return items + idx; }
  void grow(int size) {
    assert(idx + size < N);
    idx += size;
  }
  const T &back() const { return items[idx - 1]; }
  void push(const T &val) {
    assert(this->idx < (int)(sizeof(this->items) / sizeof(*this->items)));
    this->items[this->idx] = (val);
    this->idx++; /* incremented after incase `val` uses this value */
  }

  void pop() {
    assert(this->idx > 0);
    this->idx--;
  }

  void clear() { idx = 0; }
};

#define MU_COMMANDLIST_SIZE (256 * 1024)
#define MU_ROOTLIST_SIZE 32
#define MU_CONTAINERSTACK_SIZE 32
#define MU_CLIPSTACK_SIZE 32
#define MU_IDSTACK_SIZE 32
#define MU_LAYOUTSTACK_SIZE 16
#define MU_CONTAINERPOOL_SIZE 48
#define MU_TREENODEPOOL_SIZE 48

struct mu_Context {
  /* callbacks */
  int (*text_width)(mu_Font font, const char *str, int len) = nullptr;
  int (*text_height)(mu_Font font) = nullptr;
  void (*draw_frame)(mu_Context *ctx, mu_Rect rect, int colorid) = nullptr;
  /* core state */
  mu_Style _style = {};
  mu_Style *style;
  mu_Id hover;
  mu_Id last_id;
  mu_Rect last_rect;
  int last_zindex;

public:
  int frame;
  mu_Container *hover_root;
  mu_Container *next_hover_root;
  mu_Container *scroll_target;
  char number_edit_buf[MU_MAX_FMT];
  mu_Id number_edit;
  /* stacks */
  mu_Stack<char, MU_COMMANDLIST_SIZE> command_list;
  mu_Stack<mu_Container *, MU_ROOTLIST_SIZE> root_list;
  mu_Stack<mu_Container *, MU_CONTAINERSTACK_SIZE> container_stack;
  mu_Stack<mu_Rect, MU_CLIPSTACK_SIZE> clip_stack;
  mu_Stack<mu_Id, MU_IDSTACK_SIZE> id_stack;
  mu_Stack<mu_Layout, MU_LAYOUTSTACK_SIZE> layout_stack;
  /* retained state pools */
  mu_PoolItem container_pool[MU_CONTAINERPOOL_SIZE];
  mu_Container containers[MU_CONTAINERPOOL_SIZE];
  mu_PoolItem treenode_pool[MU_TREENODEPOOL_SIZE];
  /* input state */
  mu_Vec2 mouse_pos;
  mu_Vec2 last_mouse_pos;
  mu_Vec2 mouse_delta;
  mu_Vec2 scroll_delta;
  int mouse_down;
  int mouse_pressed;
  int key_down;
  int key_pressed;
  char input_text[32];

private:
  mu_Id focus = 0;
  bool updated_focus = false;

public:
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
};
