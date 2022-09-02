#pragma once
#include "mu_command.h"
#include <UIRenderFrame.h>

struct mu_Container {
  UICommandRange range;
  UIRect rect;
  UIRect body;
  UIVec2 content_size;
  UIVec2 scroll;
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
