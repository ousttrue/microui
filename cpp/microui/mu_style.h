#pragma once
#include "mu_types.h"

using text_width_callback = int (*)(mu_Font font, const char *str, int len);
using text_height_callback = int (*)(mu_Font font);

struct mu_Style {
  mu_Font font = nullptr;
  text_width_callback text_width_callback = nullptr;
  text_height_callback text_height_callback = nullptr;

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

  int text_width(const char *str, int len) const {
    return text_width_callback(font, str, len);
  }

  int text_height() const { return text_height_callback(font); }

  UIVec2 text_position(const UIRect &rect, const char *str, int len, MU_OPT opt,
                       UIVec2 *edit_size = nullptr) const {
    UIVec2 pos;
    auto tw = this->text_width(str, len);
    auto th = this->text_height();
    if (edit_size) {
      edit_size->x = tw;
      edit_size->y = th;
    }
    if (opt & MU_OPT_ALIGNCENTER) {
      pos.x = rect.x + (rect.w - tw) / 2;
    } else if (opt & MU_OPT_ALIGNRIGHT) {
      pos.x = rect.x + rect.w - tw - this->padding;
    } else {
      if (edit_size) {
        int ofx = rect.w - this->padding - tw - 1;
        pos.x = rect.x + std::min(ofx, padding);
      } else {
        pos.x = rect.x + this->padding;
      }
    }
    pos.y = rect.y + (rect.h - th) / 2;
    return pos;
  }
};
