#pragma once
#include "mu_command.h"
#include "mu_container.h"
#include "mu_hash.h"
#include "mu_input.h"
#include "mu_layout.h"
#include "mu_pool.h"
#include <assert.h>

const auto MU_MAX_FMT = 127;
const auto MU_TREENODEPOOL_SIZE = 48;

struct mu_Context {
  MuHash _hash;
  int frame = 0;
  mu_Container *scroll_target = nullptr;
  char number_edit_buf[MU_MAX_FMT] = {0};
  mu_Id number_edit = 0;
  ContainerManager _container;
  CommandDrawer _command_drawer;
  MuLayoutManager _layout;
  mu_Pool<MU_TREENODEPOOL_SIZE> treenode_pool;
  mu_Input _input;

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

  void focus_last() { _input.set_focus(_hash.last()); }

  void pop_container() {
    auto layout = _layout.pop();
    auto cnt = _container.current_container();
    cnt->content_size.x = layout->max.x - layout->body.x;
    cnt->content_size.y = layout->max.y - layout->body.y;
    _container.pop();
    _hash.pop();
  }
};
