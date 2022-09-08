#pragma once
#include "mu_clip.h"
#include "mu_stack.h"
#include "mu_style.h"
#include "mu_types.h"
#include <assert.h>
#include <string.h>

const size_t MU_COMMANDLIST_SIZE = (256 * 1024);

class CommandDrawer {
  mu_Stack<char, MU_COMMANDLIST_SIZE> _command_list;
  ClipStack _clip_stack;
  mu_Style _style = {};

public:
  mu_Style *style() { return &_style; }
  size_t size() const { return _command_list.size(); }

  void begin() {
    assert(_style.text_width_callback && _style.text_height_callback);
    _command_list.clear();
  }
  void end(UIRenderFrame *command) {
    _clip_stack.end();
    command->command_buffer = (const uint8_t *)&_command_list.get(0);
  }

  void push_clip(const UIRect &r) { _clip_stack.push(r); }
  void push_unclipped_rect() { _clip_stack.push_unclipped_rect(); }
  void pop_clip() { _clip_stack.pop(); }
  bool overlaps_clip(const UIVec2 &pos) const {
    return _clip_stack.back().overlaps_vec2(pos);
  }

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
    if (!rect.visible()) {
      return;
    }
    auto cmd = push_command(UI_COMMAND_RECT);
    cmd->rect()->rect = rect;
    cmd->rect()->color = color;
  }

  void draw_rect_color(UIRect rect, const UIColor32 &color) {
    push_rect(_clip_stack.intersect(rect), color);
  }

  void draw_rect(UIRect rect, MU_STYLE colorid) {
    draw_rect_color(rect, _style.colors[colorid]);
  }

  void draw_box(UIRect rect, MU_STYLE colorid) {
    draw_rect(UIRect(rect.x + 1, rect.y, rect.w - 2, 1), colorid);
    draw_rect(UIRect(rect.x + 1, rect.y + rect.h - 1, rect.w - 2, 1), colorid);
    draw_rect(UIRect(rect.x, rect.y, 1, rect.h), colorid);
    draw_rect(UIRect(rect.x + rect.w - 1, rect.y, 1, rect.h), colorid);
  }

  void draw_text(const char *str, int len, UIVec2 pos, MU_STYLE colorid) {
    auto color = _style.colors[colorid];
    UIRect rect =
        UIRect(pos.x, pos.y, _style.text_width(str, len), _style.text_height());
    auto clipped = _clip_stack.check_clip(rect);
    if (clipped == MU_CLIP::ALL) {
      return;
    }
    if (clipped == MU_CLIP::PART) {
      set_clip(_clip_stack.back());
    }
    // add command
    push_text(str, len, pos, color, _style.font);
    // reset clipping if it was set
    if (clipped != MU_CLIP::NONE) {
      set_clip(_clip_stack.unclipped_rect());
    }
  }

  void draw_icon(int id, UIRect rect, MU_STYLE colorid) {
    // do clip command if the rect isn't fully contained within the cliprect
    auto clipped = _clip_stack.check_clip(rect);
    if (clipped == MU_CLIP::ALL) {
      return;
    }
    if (clipped == MU_CLIP::PART) {
      set_clip(_clip_stack.back());
    }
    // do icon command
    push_icon(id, rect, _style.colors[colorid]);
    // reset clipping if it was set
    if (clipped != MU_CLIP::NONE) {
      set_clip(ClipStack::unclipped_rect());
    }
  }

  void draw_frame(UIRect rect, MU_STYLE colorid) {
    draw_rect(rect, colorid);
    if (colorid == MU_STYLE_SCROLLBASE || colorid == MU_STYLE_SCROLLTHUMB ||
        colorid == MU_STYLE_TITLEBG) {
      return;
    }
    // draw border
    if (_style.colors[MU_STYLE_BORDER].a) {
      draw_box(rect.expand(1), MU_STYLE_BORDER);
    }
  }

  void draw_control_frame(mu_Id id, UIRect rect, MU_STYLE colorid, MU_OPT opt,
                          FOCUS_STATE focus_state) {
    if (opt & MU_OPT_NOFRAME) {
      return;
    }
    colorid = static_cast<MU_STYLE>(colorid + focus_state);
    draw_frame(rect, colorid);
  }

  void draw_control_text(const char *str, const UIRect &rect, MU_STYLE colorid,
                         MU_OPT opt, bool edit = false) {
    push_clip(rect);
    if (edit) {
      UIVec2 size;
      auto pos = _style.text_position(rect, str, -1, MU_OPT_NONE, &size);
      draw_text(str, -1, pos, MU_STYLE_TEXT);
      // caret
      draw_rect(UIRect(pos.x + size.x, pos.y, 1, size.y),
                                     MU_STYLE_TEXT);
    } else {
      auto pos = _style.text_position(rect, str, -1, opt);
      draw_text(str, -1, pos, colorid);
    }
    pop_clip();
  }
};
