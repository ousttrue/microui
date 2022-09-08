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
EXPORT_MICROUI void mu_begin(mu_Context *ctx);
EXPORT_MICROUI void mu_end(mu_Context *ctx, struct UIRenderFrame *command);

//
// controls
//
EXPORT_MICROUI void mu_text(mu_Context *ctx, const char *text);
EXPORT_MICROUI void mu_label(mu_Context *ctx, const char *text);
EXPORT_MICROUI MU_RES mu_button_ex(mu_Context *ctx, const char *label, int icon,
                                   MU_OPT opt);
inline MU_RES mu_button(mu_Context *ctx, const char *label) {
  return mu_button_ex(ctx, label, 0, MU_OPT_ALIGNCENTER);
}

EXPORT_MICROUI MU_RES mu_checkbox(mu_Context *ctx, const char *label,
                                  int *state);
EXPORT_MICROUI MU_RES mu_textbox_raw(mu_Context *ctx, char *buf, int bufsz,
                                     mu_Id id, UIRect r, MU_OPT opt);
EXPORT_MICROUI MU_RES mu_textbox_ex(mu_Context *ctx, char *buf, int bufsz,
                                    MU_OPT opt);
inline MU_RES mu_textbox(mu_Context *ctx, char *buf, int bufsz) {
  return mu_textbox_ex(ctx, buf, bufsz, MU_OPT_NONE);
}

EXPORT_MICROUI MU_RES mu_slider_ex(mu_Context *ctx, mu_Real *value, mu_Real low,
                                   mu_Real high, mu_Real step, const char *fmt,
                                   MU_OPT opt);
inline MU_RES mu_slider(mu_Context *ctx, float *value, float lo, float hi) {
  return mu_slider_ex(ctx, value, lo, hi, 0, MU_SLIDER_FMT, MU_OPT_ALIGNCENTER);
}

EXPORT_MICROUI MU_RES mu_number_ex(mu_Context *ctx, mu_Real *value,
                                   mu_Real step, const char *fmt, MU_OPT opt);
inline MU_RES mu_number(mu_Context *ctx, mu_Real *value, mu_Real step) {
  return mu_number_ex(ctx, value, step, MU_SLIDER_FMT, MU_OPT_ALIGNCENTER);
}

EXPORT_MICROUI MU_RES mu_header_ex(mu_Context *ctx, const char *label,
                                   MU_OPT opt);
inline MU_RES mu_header(mu_Context *ctx, const char *label) {
  return mu_header_ex(ctx, label, MU_OPT_NONE);
}

EXPORT_MICROUI MU_RES mu_begin_treenode_ex(mu_Context *ctx, const char *label,
                                           MU_OPT opt);
inline MU_RES mu_begin_treenode(mu_Context *ctx, const char *label) {
  return mu_begin_treenode_ex(ctx, label, MU_OPT_NONE);
}

EXPORT_MICROUI void mu_end_treenode(mu_Context *ctx);
EXPORT_MICROUI MU_RES mu_begin_window(mu_Context *ctx, const char *title,
                                      UIRect rect, MU_OPT opt);
EXPORT_MICROUI void mu_end_window(mu_Context *ctx);
EXPORT_MICROUI void mu_open_popup(mu_Context *ctx, const char *name);
EXPORT_MICROUI MU_RES mu_begin_popup(mu_Context *ctx, const char *name);
EXPORT_MICROUI void mu_end_popup(mu_Context *ctx);
EXPORT_MICROUI void mu_begin_panel_ex(mu_Context *ctx, const char *name,
                                      MU_OPT opt);
inline void mu_begin_panel(mu_Context *ctx, const char *name) {
  mu_begin_panel_ex(ctx, name, MU_OPT_NONE);
}

EXPORT_MICROUI void mu_end_panel(mu_Context *ctx);

#ifdef __cplusplus
}
#endif
