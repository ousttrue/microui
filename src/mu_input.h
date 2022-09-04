#pragma once
#include <UIRenderFrame.h>

enum FOCUS_STATE {
  FOCUS_STATE_NONE,
  FOCUS_STATE_HOVER,
  FOCUS_STATE_FOCUS,
};

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

/// @brief input, focus, hover
class mu_Input {
  UIVec2 _mouse_pos;
  UIVec2 _last_mouse_pos;
  UIVec2 _mouse_delta;
  UIVec2 _scroll_delta;
  MU_MOUSE _mouse_down = MU_MOUSE_NONE;
  MU_MOUSE _mouse_pressed = MU_MOUSE_NONE;
  MU_KEY _key_down = MU_KEY_NONE;
  MU_KEY _key_pressed = MU_KEY_NONE;
  char _input_text[32] = {0};

  mu_Id _focus = 0;
  bool _keep_focus = false;
  mu_Id _hover = 0;

public:
  void begin() {
    _mouse_delta.x = _mouse_pos.x - _last_mouse_pos.x;
    _mouse_delta.y = _mouse_pos.y - _last_mouse_pos.y;
  }

  void end() {
    // unset focus if focus id was not touched this frame
    if (_keep_focus) {
      _keep_focus = false;
    } else {
      _focus = 0;
    }

    // reset input state
    this->_key_pressed = MU_KEY::MU_KEY_NONE;
    this->_input_text[0] = 0;
    this->_mouse_pressed = MU_MOUSE::MU_MOUSE_NONE;
    this->_scroll_delta = UIVec2(0, 0);
    this->_last_mouse_pos = this->_mouse_pos;
  }

  UIVec2 mouse_pos() const { return _mouse_pos; }
  UIVec2 mouse_delta() const { return _mouse_delta; }
  UIVec2 scroll_delta() const { return _scroll_delta; }
  MU_MOUSE mouse_down() const { return _mouse_down; }
  MU_MOUSE mouse_pressed() const { return _mouse_pressed; }
  const char *input_text() const { return _input_text; }
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

  void set_text(const char *text) {
    int len = strlen(this->_input_text);
    int size = strlen(text) + 1;
    assert(len + size <= (int)sizeof(this->_input_text));
    memcpy(this->_input_text + len, text, size);
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
};
