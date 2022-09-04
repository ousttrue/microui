#pragma once
#include "mu_types.h"

enum STATE {
  STATE_NONE,
  STATE_HOVER,
  STATE_FOCUS,
};

class MuFocus {
  mu_Id _focus = 0;
  bool _keep_focus = false;
  mu_Id _hover = 0;

public:
  void end() {
    if (_keep_focus) {
      _keep_focus = false;
    } else {
      _focus = 0;
    }
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

  STATE get_state(mu_Id id) const {
    return has_focus(id)  ? STATE_FOCUS
           : _hover == id ? STATE_HOVER
                          : STATE_NONE;
  }
};
