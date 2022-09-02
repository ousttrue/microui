#pragma once
#include "mu_types.h"

enum MU_STYLE {
  MU_STYLE_TEXT,
  MU_STYLE_BORDER,
  MU_STYLE_WINDOWBG,
  MU_STYLE_TITLEBG,
  MU_STYLE_TITLETEXT,
  MU_STYLE_PANELBG,
  MU_STYLE_BUTTON,
  MU_STYLE_BUTTONHOVER,
  MU_STYLE_BUTTONFOCUS,
  MU_STYLE_BASE,
  MU_STYLE_BASEHOVER,
  MU_STYLE_BASEFOCUS,
  MU_STYLE_SCROLLBASE,
  MU_STYLE_SCROLLTHUMB,
  MU_STYLE_MAX
};

struct mu_Style {
  mu_Font font = nullptr;
  UIVec2 size = {68, 10};
  int padding = 5;
  int spacing = 4;
  int indent = 24;
  int title_height = 24;
  int scrollbar_size = 12;
  int thumb_size = 8;
  UIColor32 colors[MU_STYLE_MAX] = {
      {230, 230, 230, 255}, /* MU_COLOR_TEXT */
      {25, 25, 25, 255},    /* MU_COLOR_BORDER */
      {50, 50, 50, 255},    /* MU_COLOR_WINDOWBG */
      {25, 25, 25, 255},    /* MU_COLOR_TITLEBG */
      {240, 240, 240, 255}, /* MU_COLOR_TITLETEXT */
      {0, 0, 0, 0},         /* MU_COLOR_PANELBG */
      {75, 75, 75, 255},    /* MU_COLOR_BUTTON */
      {95, 95, 95, 255},    /* MU_COLOR_BUTTONHOVER */
      {115, 115, 115, 255}, /* MU_COLOR_BUTTONFOCUS */
      {30, 30, 30, 255},    /* MU_COLOR_BASE */
      {35, 35, 35, 255},    /* MU_COLOR_BASEHOVER */
      {40, 40, 40, 255},    /* MU_COLOR_BASEFOCUS */
      {43, 43, 43, 255},    /* MU_COLOR_SCROLLBASE */
      {30, 30, 30, 255}     /* MU_COLOR_SCROLLTHUMB */
  };
};
