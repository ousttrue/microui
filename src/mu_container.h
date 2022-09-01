#pragma once
#include "mu_command.h"
#include "mu_rect.h"
#include "mu_vec2.h"

struct mu_Container {
  mu_Command *head;
  mu_Command *tail;
  mu_Rect rect;
  mu_Rect body;
  mu_Vec2 content_size;
  mu_Vec2 scroll;
  int zindex;
  int open;
};
