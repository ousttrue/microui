/*
** Copyright (c) 2020 rxi
**
** This library is free software; you can redistribute it and/or modify it
** under the terms of the MIT license. See `microui.c` for details.
*/

#pragma once
#include <type_traits>
#define MU_VERSION "2.01"

#define MU_REAL float
#define MU_REAL_FMT "%.3g"
#define MU_SLIDER_FMT "%.2f"
#define MU_MAX_FMT 127

#define mu_min(a, b) ((a) < (b) ? (a) : (b))
#define mu_max(a, b) ((a) > (b) ? (a) : (b))
#define mu_clamp(x, a, b) mu_min(b, mu_max(a, x))

enum {
  MU_ICON_CLOSE = 1,
  MU_ICON_CHECK,
  MU_ICON_COLLAPSED,
  MU_ICON_EXPANDED,
  MU_ICON_MAX
};

enum MU_RES {
  MU_RES_NONE = 0,
  MU_RES_ACTIVE = (1 << 0),
  MU_RES_SUBMIT = (1 << 1),
  MU_RES_CHANGE = (1 << 2)
};

enum MU_OPT {
  MU_OPT_NONE = 0,
  MU_OPT_ALIGNCENTER = (1 << 0),
  MU_OPT_ALIGNRIGHT = (1 << 1),
  MU_OPT_NOINTERACT = (1 << 2),
  MU_OPT_NOFRAME = (1 << 3),
  MU_OPT_NORESIZE = (1 << 4),
  MU_OPT_NOSCROLL = (1 << 5),
  MU_OPT_NOCLOSE = (1 << 6),
  MU_OPT_NOTITLE = (1 << 7),
  MU_OPT_HOLDFOCUS = (1 << 8),
  MU_OPT_AUTOSIZE = (1 << 9),
  MU_OPT_POPUP = (1 << 10),
  MU_OPT_CLOSED = (1 << 11),
  MU_OPT_EXPANDED = (1 << 12)
};
inline MU_OPT operator|(MU_OPT L, MU_OPT R) {
  return static_cast<MU_OPT>(
      static_cast<std::underlying_type<MU_OPT>::type>(L) |
      static_cast<std::underlying_type<MU_OPT>::type>(R));
}
inline MU_OPT operator&(MU_OPT L, MU_OPT R) {
  return static_cast<MU_OPT>(
      static_cast<std::underlying_type<MU_OPT>::type>(L) &
      static_cast<std::underlying_type<MU_OPT>::type>(R));
}

enum {
  MU_MOUSE_LEFT = (1 << 0),
  MU_MOUSE_RIGHT = (1 << 1),
  MU_MOUSE_MIDDLE = (1 << 2)
};

enum {
  MU_KEY_SHIFT = (1 << 0),
  MU_KEY_CTRL = (1 << 1),
  MU_KEY_ALT = (1 << 2),
  MU_KEY_BACKSPACE = (1 << 3),
  MU_KEY_RETURN = (1 << 4)
};

typedef unsigned mu_Id;
typedef MU_REAL mu_Real;

struct mu_PoolItem {
  mu_Id id;
  int last_update;
};

#include "mu_context.h"

void mu_begin(mu_Context *ctx);
void mu_end(mu_Context *ctx);

mu_Id mu_get_id(mu_Context *ctx, const void *data, int size);
void mu_push_id(mu_Context *ctx, const void *data, int size);
void mu_pop_id(mu_Context *ctx);

mu_Container *mu_get_current_container(mu_Context *ctx);
mu_Container *mu_get_container(mu_Context *ctx, const char *name);
void mu_bring_to_front(mu_Context *ctx, mu_Container *cnt);

int mu_pool_init(mu_Context *ctx, mu_PoolItem *items, int len, mu_Id id);
int mu_pool_get(mu_Context *ctx, mu_PoolItem *items, int len, mu_Id id);
void mu_pool_update(mu_Context *ctx, mu_PoolItem *items, int idx);

void mu_input_mousemove(mu_Context *ctx, int x, int y);
void mu_input_mousedown(mu_Context *ctx, int x, int y, int btn);
void mu_input_mouseup(mu_Context *ctx, int x, int y, int btn);
void mu_input_scroll(mu_Context *ctx, int x, int y);
void mu_input_keydown(mu_Context *ctx, int key);
void mu_input_keyup(mu_Context *ctx, int key);
void mu_input_text(mu_Context *ctx, const char *text);

void mu_draw_text(mu_Context *ctx, mu_Font font, const char *str, int len,
                  mu_Vec2 pos, mu_Color color);
void mu_draw_icon(mu_Context *ctx, int id, mu_Rect rect, mu_Color color);

void mu_layout_width(mu_Context *ctx, int width);
void mu_layout_height(mu_Context *ctx, int height);
void mu_layout_begin_column(mu_Context *ctx);
void mu_layout_end_column(mu_Context *ctx);
void mu_layout_set_next(mu_Context *ctx, mu_Rect r, int relative);
mu_Rect mu_layout_next(mu_Context *ctx);

void mu_draw_control_frame(mu_Context *ctx, mu_Id id, mu_Rect rect, int colorid,
                           MU_OPT opt);
void mu_draw_control_text(mu_Context *ctx, const char *str, mu_Rect rect,
                          int colorid, MU_OPT opt);
int mu_mouse_over(mu_Context *ctx, mu_Rect rect);
void mu_update_control(mu_Context *ctx, mu_Id id, mu_Rect rect, MU_OPT opt);

#define mu_button(ctx, label) mu_button_ex(ctx, label, 0, MU_OPT_ALIGNCENTER)
#define mu_textbox(ctx, buf, bufsz) mu_textbox_ex(ctx, buf, bufsz, MU_OPT_NONE)
#define mu_slider(ctx, value, lo, hi)                                          \
  mu_slider_ex(ctx, value, lo, hi, 0, MU_SLIDER_FMT, MU_OPT_ALIGNCENTER)
#define mu_number(ctx, value, step)                                            \
  mu_number_ex(ctx, value, step, MU_SLIDER_FMT, MU_OPT_ALIGNCENTER)
#define mu_header(ctx, label) mu_header_ex(ctx, label, MU_OPT_NONE)
#define mu_begin_treenode(ctx, label) mu_begin_treenode_ex(ctx, label, MU_OPT_NONE)
#define mu_begin_panel(ctx, name) mu_begin_panel_ex(ctx, name, MU_OPT_NONE)

void mu_text(mu_Context *ctx, const char *text);
void mu_label(mu_Context *ctx, const char *text);
int mu_button_ex(mu_Context *ctx, const char *label, int icon, MU_OPT opt);
int mu_checkbox(mu_Context *ctx, const char *label, int *state);
int mu_textbox_raw(mu_Context *ctx, char *buf, int bufsz, mu_Id id, mu_Rect r,
                   MU_OPT opt);
int mu_textbox_ex(mu_Context *ctx, char *buf, int bufsz, MU_OPT opt);
int mu_slider_ex(mu_Context *ctx, mu_Real *value, mu_Real low, mu_Real high,
                 mu_Real step, const char *fmt, MU_OPT opt);
int mu_number_ex(mu_Context *ctx, mu_Real *value, mu_Real step, const char *fmt,
                 MU_OPT opt);
int mu_header_ex(mu_Context *ctx, const char *label, MU_OPT opt);
int mu_begin_treenode_ex(mu_Context *ctx, const char *label, MU_OPT opt);
void mu_end_treenode(mu_Context *ctx);
int mu_begin_window(mu_Context *ctx, const char *title, mu_Rect rect,
                    MU_OPT opt = MU_OPT_NONE);
void mu_end_window(mu_Context *ctx);
void mu_open_popup(mu_Context *ctx, const char *name);
int mu_begin_popup(mu_Context *ctx, const char *name);
void mu_end_popup(mu_Context *ctx);
void mu_begin_panel_ex(mu_Context *ctx, const char *name, MU_OPT opt);
void mu_end_panel(mu_Context *ctx);
