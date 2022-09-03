/*
** Copyright (c) 2020 rxi
**
** This library is free software; you can redistribute it and/or modify it
** under the terms of the MIT license. See `microui.c` for details.
*/

#pragma once
#include "mu_types.h"
#include <UIRenderFrame.h>
#define MU_VERSION "2.01"

#define MU_REAL_FMT "%.3g"
#define MU_SLIDER_FMT "%.2f"

#define mu_min(a, b) ((a) < (b) ? (a) : (b))
#define mu_max(a, b) ((a) > (b) ? (a) : (b))
#define mu_clamp(x, a, b) mu_min(b, mu_max(a, x))

enum MU_ICON {
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


struct mu_Context;
struct mu_Container;
struct mu_PoolItem;
#ifdef __cplusplus
#else
typedef struct mu_Context mu_Context;
typedef struct mu_Container mu_Container;
typedef struct mu_PoolItem mu_PoolItem;
typedef struct UIVec2 UIVec2;
typedef struct UIRect UIRect;
typedef struct UIColor32 UIColor32;
typedef enum MU_OPT MU_OPT;
typedef enum MU_RES MU_RES;
#endif

#ifdef BUILD_MICROUI
#define EXPORT_MICROUI __declspec(dllexport)
#else
#define EXPORT_MICROUI __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT_MICROUI mu_Context *mu_new(int (*text_width)(void *, const char *, int),
                                  int (*text_height)(void *));
EXPORT_MICROUI void mu_delete(mu_Context *ctx);

EXPORT_MICROUI void mu_input_mousemove(mu_Context *ctx, int x, int y);
EXPORT_MICROUI void mu_input_mousedown(mu_Context *ctx, int button);
EXPORT_MICROUI void mu_input_mouseup(mu_Context *ctx, int button);
EXPORT_MICROUI void mu_input_scroll(mu_Context *ctx, int x, int y);

EXPORT_MICROUI void mu_begin(mu_Context *ctx);
EXPORT_MICROUI void mu_end(mu_Context *ctx, struct UIRenderFrame *command);

EXPORT_MICROUI mu_Id mu_get_id(mu_Context *ctx, const void *data, int size);
EXPORT_MICROUI void mu_push_id(mu_Context *ctx, const void *data, int size);
EXPORT_MICROUI void mu_pop_id(mu_Context *ctx);

EXPORT_MICROUI mu_Container *mu_get_current_container(mu_Context *ctx);
EXPORT_MICROUI mu_Container *mu_get_container(mu_Context *ctx,
                                              const char *name);

EXPORT_MICROUI int mu_pool_init(mu_Context *ctx, mu_PoolItem *items, int len,
                                mu_Id id);
EXPORT_MICROUI int mu_pool_get(mu_PoolItem *items, int len, mu_Id id);
EXPORT_MICROUI void mu_pool_update(int frame, mu_PoolItem *items, int idx);

EXPORT_MICROUI void mu_draw_text(mu_Context *ctx, mu_Font font, const char *str,
                                 int len, UIVec2 pos, UIColor32 color);
EXPORT_MICROUI void mu_draw_icon(mu_Context *ctx, int id, UIRect rect,
                                 UIColor32 color);

EXPORT_MICROUI void mu_layout_width(mu_Context *ctx, int width);
EXPORT_MICROUI void mu_layout_height(mu_Context *ctx, int height);
EXPORT_MICROUI void mu_layout_begin_column(mu_Context *ctx);
EXPORT_MICROUI void mu_layout_end_column(mu_Context *ctx);
EXPORT_MICROUI void mu_layout_set_next(mu_Context *ctx, UIRect r,
                                       int relative);
EXPORT_MICROUI UIRect mu_layout_next(mu_Context *ctx);

EXPORT_MICROUI void mu_draw_control_frame(mu_Context *ctx, mu_Id id,
                                          UIRect rect, int colorid,
                                          MU_OPT opt);
EXPORT_MICROUI void mu_draw_control_text(mu_Context *ctx, const char *str,
                                         UIRect rect, int colorid, MU_OPT opt);
EXPORT_MICROUI int mu_mouse_over(mu_Context *ctx, UIRect rect);
EXPORT_MICROUI void mu_update_control(mu_Context *ctx, mu_Id id, UIRect rect,
                                      MU_OPT opt);

#define mu_button(ctx, label) mu_button_ex(ctx, label, 0, MU_OPT_ALIGNCENTER)
#define mu_textbox(ctx, buf, bufsz) mu_textbox_ex(ctx, buf, bufsz, MU_OPT_NONE)
#define mu_slider(ctx, value, lo, hi)                                          \
  mu_slider_ex(ctx, value, lo, hi, 0, MU_SLIDER_FMT, MU_OPT_ALIGNCENTER)
#define mu_number(ctx, value, step)                                            \
  mu_number_ex(ctx, value, step, MU_SLIDER_FMT, MU_OPT_ALIGNCENTER)
#define mu_header(ctx, label) mu_header_ex(ctx, label, MU_OPT_NONE)
#define mu_begin_treenode(ctx, label)                                          \
  mu_begin_treenode_ex(ctx, label, MU_OPT_NONE)
#define mu_begin_panel(ctx, name) mu_begin_panel_ex(ctx, name, MU_OPT_NONE)

EXPORT_MICROUI void mu_text(mu_Context *ctx, const char *text);
EXPORT_MICROUI void mu_label(mu_Context *ctx, const char *text);
EXPORT_MICROUI MU_RES mu_button_ex(mu_Context *ctx, const char *label, int icon,
                                   MU_OPT opt);
EXPORT_MICROUI MU_RES mu_checkbox(mu_Context *ctx, const char *label,
                                  int *state);
EXPORT_MICROUI MU_RES mu_textbox_raw(mu_Context *ctx, char *buf, int bufsz,
                                     mu_Id id, UIRect r, MU_OPT opt);
EXPORT_MICROUI MU_RES mu_textbox_ex(mu_Context *ctx, char *buf, int bufsz,
                                    MU_OPT opt);
EXPORT_MICROUI MU_RES mu_slider_ex(mu_Context *ctx, mu_Real *value, mu_Real low,
                                   mu_Real high, mu_Real step, const char *fmt,
                                   MU_OPT opt);
EXPORT_MICROUI MU_RES mu_number_ex(mu_Context *ctx, mu_Real *value,
                                   mu_Real step, const char *fmt, MU_OPT opt);
EXPORT_MICROUI MU_RES mu_header_ex(mu_Context *ctx, const char *label,
                                   MU_OPT opt);
EXPORT_MICROUI MU_RES mu_begin_treenode_ex(mu_Context *ctx, const char *label,
                                           MU_OPT opt);
EXPORT_MICROUI void mu_end_treenode(mu_Context *ctx);
EXPORT_MICROUI MU_RES mu_begin_window(mu_Context *ctx, const char *title,
                                      UIRect rect, MU_OPT opt);
EXPORT_MICROUI void mu_end_window(mu_Context *ctx);
EXPORT_MICROUI void mu_open_popup(mu_Context *ctx, const char *name);
EXPORT_MICROUI MU_RES mu_begin_popup(mu_Context *ctx, const char *name);
EXPORT_MICROUI void mu_end_popup(mu_Context *ctx);
EXPORT_MICROUI void mu_begin_panel_ex(mu_Context *ctx, const char *name,
                                      MU_OPT opt);
EXPORT_MICROUI void mu_end_panel(mu_Context *ctx);

#ifdef __cplusplus
}
#endif
