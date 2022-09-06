#pragma once
#include "mu_types.h"
#include <UIRenderFrame.h>
#include <string>

enum MU_MOUSE {
  MU_MOUSE_NONE,
  MU_MOUSE_LEFT = (1 << 0),
  MU_MOUSE_RIGHT = (1 << 1),
  MU_MOUSE_MIDDLE = (1 << 2)
};
inline MU_MOUSE operator|(MU_MOUSE L, MU_MOUSE R) {
  return static_cast<MU_MOUSE>(
      static_cast<std::underlying_type<MU_MOUSE>::type>(L) |
      static_cast<std::underlying_type<MU_MOUSE>::type>(R));
}
inline MU_MOUSE operator&(MU_MOUSE L, MU_MOUSE R) {
  return static_cast<MU_MOUSE>(
      static_cast<std::underlying_type<MU_MOUSE>::type>(L) &
      static_cast<std::underlying_type<MU_MOUSE>::type>(R));
}

enum MU_KEY {
  MU_KEY_NONE,
  MU_KEY_SHIFT = (1 << 0),
  MU_KEY_CTRL = (1 << 1),
  MU_KEY_ALT = (1 << 2),
  MU_KEY_BACKSPACE = (1 << 3),
  MU_KEY_RETURN = (1 << 4)
};
inline MU_KEY operator|(MU_KEY L, MU_KEY R) {
  return static_cast<MU_KEY>(
      static_cast<std::underlying_type<MU_KEY>::type>(L) |
      static_cast<std::underlying_type<MU_KEY>::type>(R));
}
inline MU_KEY operator&(MU_KEY L, MU_KEY R) {
  return static_cast<MU_KEY>(
      static_cast<std::underlying_type<MU_KEY>::type>(L) &
      static_cast<std::underlying_type<MU_KEY>::type>(R));
}

const auto INPUT_TEXT_SIZE = 32;
/// @brief input, focus, hover
struct mu_Container;
class mu_Input {
  UIVec2 _mouse_pos;
  UIVec2 _last_mouse_pos;
  UIVec2 _mouse_delta;
  UIVec2 _scroll_delta;
  MU_MOUSE _mouse_down = MU_MOUSE_NONE;
  MU_MOUSE _mouse_pressed = MU_MOUSE_NONE;
  MU_KEY _key_down = MU_KEY_NONE;
  MU_KEY _key_pressed = MU_KEY_NONE;
  char _input_text[INPUT_TEXT_SIZE] = {0};
  uint32_t _input_text_pos = 0;

  mu_Id _focus = 0;
  bool _keep_focus = false;
  mu_Id _hover = 0;

  mu_Container *_scroll_target = nullptr;

public:
  void begin() {
    _mouse_delta.x = _mouse_pos.x - _last_mouse_pos.x;
    _mouse_delta.y = _mouse_pos.y - _last_mouse_pos.y;
    _scroll_target = nullptr;
  }

  bool end() {
    // unset focus if focus id was not touched this frame
    if (_keep_focus) {
      _keep_focus = false;
    } else {
      _focus = 0;
    }

    // handle scroll input
    auto mouse_pressed = _mouse_pressed;
    if (_scroll_target) {
      _scroll_target->scroll += scroll_delta();
    }

    // reset input state
    _key_pressed = MU_KEY::MU_KEY_NONE;
    _input_text_pos = 0;
    _mouse_pressed = MU_MOUSE::MU_MOUSE_NONE;
    _scroll_delta = UIVec2(0, 0);
    _last_mouse_pos = _mouse_pos;

    return mouse_pressed;
  }

  void set_scroll_target(mu_Container *cnt) { _scroll_target = cnt; }

  UIVec2 mouse_pos() const { return _mouse_pos; }
  UIVec2 mouse_delta() const { return _mouse_delta; }
  UIVec2 scroll_delta() const { return _scroll_delta; }
  MU_MOUSE mouse_down() const { return _mouse_down; }
  MU_MOUSE mouse_pressed() const { return _mouse_pressed; }
  std::u32string input_text() const {
    return {_input_text, _input_text + _input_text_pos};
  }
  MU_KEY key_down() const { return _key_down; }
  MU_KEY key_pressed() const { return _key_pressed; }

  void set_mousemove(int x, int y) { this->_mouse_pos = UIVec2(x, y); }

  void set_mousedown(MU_MOUSE btn) {
    this->_mouse_down = this->_mouse_down | btn;
    this->_mouse_pressed = this->_mouse_pressed | btn;
  }

  void set_mouseup(MU_MOUSE btn) {
    this->_mouse_down = this->_mouse_down & static_cast<MU_MOUSE>(~btn);
  }

  void add_scroll_delta(int x, int y) {
    this->_scroll_delta.x += x;
    this->_scroll_delta.y += y;
  }

  void set_keydown(MU_KEY key) {
    this->_key_pressed = this->_key_pressed | key;
    this->_key_down = this->_key_down | key;
  }

  void set_keyup(MU_KEY key) {
    this->_key_down = this->_key_down & static_cast<MU_KEY>(~key);
  }

  void push_utf8(const char *p, int size) {
    for (int i = 0; i < size; ++i, ++p) {
      this->_input_text[_input_text_pos++] = *p;
    }
    if (_input_text_pos < INPUT_TEXT_SIZE) {
      this->_input_text[_input_text_pos] = 0;
    }
  }

  int consume_text(char *buf, int bufsz) {
    int n = mu_min(bufsz - 1, _input_text_pos);
    if (n > 0) {
      memcpy(buf, _input_text, n);
    }
    if (n < bufsz) {
      buf[n] = 0;
    }
    _input_text_pos = 0;
    return n;
  }

  void set_focus(mu_Id id) {
    _focus = id;
    _keep_focus = true;
  }
  void keep_focus(mu_Id id) {
    if (_focus == id) {
      _keep_focus = true;
    }
  }
  bool has_focus(mu_Id id) const { return _focus == id; }
  void set_hover(mu_Id id) { _hover = id; }
  bool has_hover(mu_Id id) const { return _hover == id; }

  FOCUS_STATE get_focus_state(mu_Id id) const {
    return has_focus(id)  ? FOCUS_STATE_FOCUS
           : _hover == id ? FOCUS_STATE_HOVER
                          : FOCUS_STATE_NONE;
  }

  void update_focus_hover(mu_Id id, MU_OPT opt, bool mouseover) {
    this->keep_focus(id);
    if (opt & MU_OPT_NOINTERACT) {
      return;
    }

    if (mouseover && !this->mouse_down()) {
      this->set_hover(id);
    }

    if (this->has_focus(id)) {
      if (this->mouse_pressed() && !mouseover) {
        this->set_focus(0);
      }
      if (!this->mouse_down() && ~opt & MU_OPT_HOLDFOCUS) {
        this->set_focus(0);
      }
    }

    if (this->has_hover(id)) {
      if (this->mouse_pressed()) {
        this->set_focus(id);
      } else if (!mouseover) {
        this->set_hover(0);
      }
    }
  }
};
