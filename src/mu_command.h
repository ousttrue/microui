#pragma once
#include "mu_color.h"
#include "mu_rect.h"
#include "mu_stack.h"
#include "mu_types.h"
#include "mu_vec2.h"
#include <string.h>

enum class MU_COMMAND { JUMP = 1, CLIP, RECT, TEXT, ICON, MAX };

struct mu_BaseCommand {
  MU_COMMAND type;
  int size;
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
  MU_COMMAND type;
  mu_BaseCommand base;
  mu_JumpCommand jump;
  mu_ClipCommand clip;
  mu_RectCommand rect;
  mu_TextCommand text;
  mu_IconCommand icon;
};

#define MU_COMMANDLIST_SIZE (256 * 1024)

class CommandStack {
public:
  mu_Stack<char, MU_COMMANDLIST_SIZE> _command_list;

  void begin_frame() { _command_list.clear(); }

  mu_Command *push_command(MU_COMMAND type, int size) {
    mu_Command *cmd = (mu_Command *)(this->_command_list.end());
    cmd->base.type = type;
    cmd->base.size = size;
    this->_command_list.grow(size);
    return cmd;
  }

  mu_Command *push_jump(mu_Command *dst) {
    auto cmd = push_command(MU_COMMAND::JUMP, sizeof(mu_JumpCommand));
    cmd->jump.dst = dst;
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

  int mu_next_command(mu_Command **cmd) {
    if (*cmd) {
      *cmd = (mu_Command *)(((char *)*cmd) + (*cmd)->base.size);
    } else {
      *cmd = (mu_Command *)_command_list.begin();
    }
    while ((char *)*cmd != _command_list.end()) {
      if ((*cmd)->type != MU_COMMAND::JUMP) {
        return 1;
      }
      *cmd = (mu_Command *)((*cmd)->jump.dst);
    }
    return 0;
  }
};
