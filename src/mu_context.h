#pragma once
#include "mu_command.h"
#include "mu_container.h"
#include "mu_hash.h"
#include "mu_input.h"
#include "mu_layout.h"
#include "mu_pool.h"
#include "mu_style.h"
#include <assert.h>

const auto MU_MAX_FMT = 127;
const auto MU_LAYOUTSTACK_SIZE = 16;
const auto MU_TREENODEPOOL_SIZE = 48;

struct mu_Context {
  MuHash _hash;
  mu_Style _style = {};
  mu_Style *style = nullptr;
  UIRect last_rect;
  int frame = 0;
  mu_Container *scroll_target = nullptr;
  char number_edit_buf[MU_MAX_FMT] = {0};
  mu_Id number_edit = 0;
  ContainerManager _container;
  CommandDrawer _command_drawer;
  mu_Stack<mu_Layout, MU_LAYOUTSTACK_SIZE> layout_stack;
  mu_Pool<MU_TREENODEPOOL_SIZE> treenode_pool;
  mu_Input _input;

public:
  mu_Context() {
    this->_style = {};
    this->style = &this->_style;
  }
  // avoid copy
  mu_Context(const mu_Context &) = delete;
  mu_Context &operator=(const mu_Context &) = delete;

  bool mouse_over(const UIRect &rect) const {
    if (!rect.overlaps_vec2(this->_input.mouse_pos())) {
      return false;
    }
    if (!this->_command_drawer.overlaps_clip(this->_input.mouse_pos())) {
      return false;
    }
    if (!this->_container.in_hover_root()) {
      return false;
    }
    return true;
  }

  void focus_last() { _input.set_focus(_hash.last()); }
};
