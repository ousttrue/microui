#pragma once
#include "mu_command.h"
#include "mu_container.h"
#include "mu_hash.h"
#include "mu_input.h"
#include "mu_layout.h"
#include "mu_pool.h"
#include "mu_tree.h"
#include <assert.h>

const auto MU_MAX_FMT = 127;

struct mu_Context {
  MuHash _hash;
  int frame = 0;
  ContainerManager _container;
  CommandDrawer _command_drawer;
  MuLayoutManager _layout;
  mu_Input _input;
  TreeManager _tree;
  char number_edit_buf[MU_MAX_FMT] = {0};
  mu_Id number_edit = 0;

public:
  // avoid copy
  mu_Context() {}
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
};
