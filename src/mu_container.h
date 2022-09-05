#pragma once
#include "mu_pool.h"
#include "mu_stack.h"
#include <UIRenderFrame.h>
#include <stdlib.h>

struct mu_Container {
  UICommandRange range;
  UIRect rect;
  UIRect body;
  UIVec2 content_size;
  UIVec2 scroll;
  int zindex;
  bool open;

  void reset() {
    range.head = 0;
    range.tail = 0;
    rect = {};
    body = {};
    content_size = {};
    scroll = {};
    zindex = 0;
    open = true;
  }
};

const auto MU_CONTAINERSTACK_SIZE = 32;
const auto MU_CONTAINERPOOL_SIZE = 48;
const auto MU_ROOTLIST_SIZE = 32;

class ContainerManager {
  // container pool managment
  mu_Pool<MU_CONTAINERPOOL_SIZE> _container_pool;
  mu_Container _containers[MU_CONTAINERPOOL_SIZE] = {0};

  // container stack
  mu_Stack<mu_Container *, MU_CONTAINERSTACK_SIZE> _container_stack;

  // root window management
  int _last_zindex = 0;
  mu_Container *_hover_root = nullptr;
  mu_Container *_next_hover_root = nullptr;
  mu_Stack<mu_Container *, MU_ROOTLIST_SIZE> _root_list;
  UICommandRange _root_window_ranges[MU_ROOTLIST_SIZE];

  static int compare_zindex(const void *a, const void *b) {
    return (*(mu_Container **)a)->zindex - (*(mu_Container **)b)->zindex;
  }

public:
  void begin() {
    _hover_root = _next_hover_root;
    _next_hover_root = nullptr;
    _root_list.clear();
  }
  void end(bool mouse_pressed, UIRenderFrame *command) {
    assert(_container_stack.size() == 0);

    // bring hover root to front if mouse was pressed
    if (mouse_pressed) {
      if (this->_next_hover_root &&
          // this->next_hover_root->zindex < this->last_zindex &&
          this->_next_hover_root->zindex >= 0) {
        bring_to_front(this->_next_hover_root);
      }
    }

    // sort root containers by zindex
    auto n = _root_list.size();
    ::qsort((void *)_root_list.begin(), n, sizeof(mu_Container *),
            compare_zindex);

    auto end = _root_list.end();
    auto p = &_root_window_ranges[0];
    for (auto it = _root_list.begin(); it != end; ++it, ++p) {
      *p = (*it)->range;
    }

    command->command_groups = &_root_window_ranges[0];
    command->command_group_count = _root_list.size();
  }
  void bring_to_front(mu_Container *cnt) { cnt->zindex = ++this->_last_zindex; }
  bool is_hover_root(const mu_Container *cnt) const {
    return _hover_root == cnt;
  }

  mu_Container *get_container(mu_Id id, MU_OPT opt, int frame) {
    // try to get existing container from pool
    {
      int idx = _container_pool.get_index(id);
      if (idx >= 0) {
        if (_containers[idx].open || ~opt & MU_OPT_CLOSED) {
          _container_pool.update(frame, idx);
        }
        return &_containers[idx];
      }
    }

    if (opt & MU_OPT_CLOSED) {
      return nullptr;
    }

    // container not found in pool: init new container
    auto idx = _container_pool.init(frame, id);
    auto cnt = &_containers[idx];
    cnt->reset();
    bring_to_front(cnt);
    return cnt;
  }

  void push(mu_Container *cnt) { _container_stack.push(cnt); }

  mu_Container *current_container() { return _container_stack.back(); }

  void pop() {
    // pop container, layout and id
    _container_stack.pop();
  }

  bool in_hover_root() const {
    int i = _container_stack.size();
    while (i--) {
      if (_container_stack.get(i) == _hover_root) {
        return true;
      }
      /* only root containers have their `head` field set; stop searching if
       *we've
       ** reached the current root container */
      if (_container_stack.get(i)->range.head) {
        break;
      }
    }
    return false;
  }

  void begin_root_container(mu_Container *cnt, size_t command_head,
                            const UIVec2 &mouse_pos) {
    _container_stack.push(cnt);
    // push container to roots list and push head command
    _root_list.push(cnt);
    cnt->range.head = command_head;
    /* set as hover root if the mouse is overlapping this container and it has a
    ** higher zindex than the current hover root */
    if (cnt->rect.overlaps_vec2(mouse_pos) &&
        (!_next_hover_root || cnt->zindex > _next_hover_root->zindex)) {
      _next_hover_root = cnt;
    }
  }

  void open_popup(mu_Id id, const UIVec2 &mouse_pos, int frame) {
    auto cnt = get_container(id, MU_OPT::MU_OPT_NONE, frame);
    // set as hover root so popup isn't closed in begin_window_ex()
    _hover_root = _next_hover_root = cnt;
    // position at mouse cursor, open and bring-to-front
    cnt->rect = UIRect(mouse_pos.x, mouse_pos.y, 1, 1);
    cnt->open = true;
    bring_to_front(cnt);
  }
};
