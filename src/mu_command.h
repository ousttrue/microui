#pragma once
#include "mu_color.h"
#include "mu_rect.h"
#include "mu_vec2.h"

struct mu_BaseCommand {
  int type, size;
};
struct mu_JumpCommand {
  mu_BaseCommand base;
  void *dst;
};
struct mu_ClipCommand {
  mu_BaseCommand base;
  mu_Rect rect;
};
struct mu_RectCommand {
  mu_BaseCommand base;
  mu_Rect rect;
  mu_Color color;
};
struct mu_TextCommand {
  mu_BaseCommand base;
  mu_Font font;
  mu_Vec2 pos;
  mu_Color color;
  char str[1];
};
struct mu_IconCommand {
  mu_BaseCommand base;
  mu_Rect rect;
  int id;
  mu_Color color;
};

union mu_Command {
  int type;
  mu_BaseCommand base;
  mu_JumpCommand jump;
  mu_ClipCommand clip;
  mu_RectCommand rect;
  mu_TextCommand text;
  mu_IconCommand icon;
};
