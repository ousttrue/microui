#pragma once
#include "mu_color.h"
#include "mu_rect.h"
#include "mu_stack.h"
#include "mu_types.h"
#include "mu_vec2.h"
#include <string.h>

enum class MU_COMMAND : unsigned int { CLIP, RECT, TEXT, ICON };

struct mu_ClipCommand {
  mu_Rect rect;
};
struct mu_RectCommand {
  mu_Rect rect;
  mu_Color color;
};
struct mu_TextCommand {
  mu_Font font;
  mu_Vec2 pos;
  mu_Color color;
  char str[1];
};
struct mu_IconCommand {
  mu_Rect rect;
  int id;
  mu_Color color;
};

struct mu_Command {
  MU_COMMAND type;
  int size;
  union {
    mu_ClipCommand clip;
    mu_RectCommand rect;
    mu_TextCommand text;
    mu_IconCommand icon;
  };
};

const size_t MU_COMMANDLIST_SIZE = (256 * 1024);

class CommandStack {
  mu_Stack<char, MU_COMMANDLIST_SIZE> _command_list;

public:
  size_t size() const { return _command_list.size(); }
  char *get(size_t i) { return &_command_list.get(i); }

  void begin_frame() { _command_list.clear(); }

  mu_Command *push_command(MU_COMMAND type, int size) {
    auto cmd = (mu_Command *)(this->_command_list.end());
    cmd->type = type;
    cmd->size = size + 8;
    this->_command_list.grow(size + 8);
    return cmd;
  }

  void set_clip(mu_Rect rect) {
    auto cmd = push_command(MU_COMMAND::CLIP, sizeof(mu_ClipCommand));
    cmd->clip.rect = rect;
  }

  void push_text(const char *str, int len, const mu_Vec2 &pos,
                 const mu_Color &color, const mu_Font &font) {
    if (len < 0) {
      len = strlen(str);
    }
    auto cmd = push_command(MU_COMMAND::TEXT, sizeof(mu_TextCommand) + len);
    memcpy(cmd->text.str, str, len);
    cmd->text.str[len] = '\0';
    cmd->text.pos = pos;
    cmd->text.color = color;
    cmd->text.font = font;
  }

  void push_icon(int id, const mu_Rect &rect, const mu_Color &color) {
    auto cmd = push_command(MU_COMMAND::ICON, sizeof(mu_IconCommand));
    cmd->icon.id = id;
    cmd->icon.rect = rect;
    cmd->icon.color = color;
  }

  void push_rect(const mu_Rect &rect, const mu_Color &color) {
    auto cmd = push_command(MU_COMMAND::RECT, sizeof(mu_RectCommand));
    cmd->rect.rect = rect;
    cmd->rect.color = color;
  }
};
