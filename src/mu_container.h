#pragma once
#include "mu_command.h"
#include "mu_rect.h"
#include "mu_vec2.h"
#include "mu_render_command.h"

struct mu_Container {
  mu_WindowRange range;
  mu_Rect rect;
  mu_Rect body;
  mu_Vec2 content_size;
  mu_Vec2 scroll;
  int zindex;
  bool open;

  void init() {
    range.head = 0;
    range.tail = 0;
    rect = {};
    body = {};
    content_size = {};
    scroll = {};
    zindex = 0;
    open = true;
  }
};
