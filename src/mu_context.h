#pragma once
#include "mu_container.h"
#include "mu_rect.h"
#include "mu_style.h"
#include "mu_layout.h"

#define mu_stack(T, n)                                                         \
  struct {                                                                     \
    int idx;                                                                   \
    T items[n];                                                                \
  }
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
  int (*text_width)(mu_Font font, const char *str, int len);
  int (*text_height)(mu_Font font);
  void (*draw_frame)(mu_Context *ctx, mu_Rect rect, int colorid);
  /* core state */
  mu_Style _style;
  mu_Style *style;
  mu_Id hover;
  mu_Id focus;
  mu_Id last_id;
  mu_Rect last_rect;
  int last_zindex;
  int updated_focus;
  int frame;
  mu_Container *hover_root;
  mu_Container *next_hover_root;
  mu_Container *scroll_target;
  char number_edit_buf[MU_MAX_FMT];
  mu_Id number_edit;
  /* stacks */
  mu_stack(char, MU_COMMANDLIST_SIZE) command_list;
  mu_stack(mu_Container *, MU_ROOTLIST_SIZE) root_list;
  mu_stack(mu_Container *, MU_CONTAINERSTACK_SIZE) container_stack;
  mu_stack(mu_Rect, MU_CLIPSTACK_SIZE) clip_stack;
  mu_stack(mu_Id, MU_IDSTACK_SIZE) id_stack;
  mu_stack(mu_Layout, MU_LAYOUTSTACK_SIZE) layout_stack;
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
};
