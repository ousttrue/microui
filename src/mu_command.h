#pragma once
#include "mu_stack.h"
#include "mu_types.h"
#include <string.h>

const size_t MU_COMMANDLIST_SIZE = (256 * 1024);

class CommandStack {
  mu_Stack<char, MU_COMMANDLIST_SIZE> _command_list;

public:
  size_t size() const { return _command_list.size(); }
  char *get(size_t i) { return &_command_list.get(i); }

  void begin_frame() { _command_list.clear(); }

  UICommandHeader *push_command(UI_COMMAND type, size_t text_length = 0) {
    auto cmd = reinterpret_cast<UICommandHeader *>(this->_command_list.end());
    this->_command_list.grow(cmd->set_type(type, text_length));
    return cmd;
  }

  void set_clip(const UIRect &rect) {
    auto cmd = push_command(UI_COMMAND_CLIP);
    cmd->clip()->rect = rect;
  }

  void push_text(const char *str, int len, const UIVec2 &pos,
                 const UIColor32 &color, const void *font) {
    if (len < 0) {
      len = strlen(str);
    }
    auto cmd = push_command(UI_COMMAND_TEXT, len);
    memcpy((char *)cmd->text()->begin(), str, len);
    cmd->text()->pos = pos;
    cmd->text()->color = color;
    cmd->text()->font = font;
  }

  void push_icon(int id, const UIRect &rect, const UIColor32 &color) {
    auto cmd = push_command(UI_COMMAND_ICON);
    cmd->icon()->id = id;
    cmd->icon()->rect = rect;
    cmd->icon()->color = color;
  }

  void push_rect(const UIRect &rect, const UIColor32 &color) {
    auto cmd = push_command(UI_COMMAND_RECT);
    cmd->rect()->rect = rect;
    cmd->rect()->color = color;
  }
};
