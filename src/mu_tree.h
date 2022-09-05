#pragma once
#include "mu_stack.h"
#include "mu_types.h"

const auto MU_TREENODEPOOL_SIZE = 48;

class TreeManager {
  mu_Pool<MU_TREENODEPOOL_SIZE> _treenode_pool;

public:
  int get(mu_Id id) const { return _treenode_pool.get_index(id); }

  void update(mu_Id id, int idx, bool active, int frame) {
    if (idx >= 0) {
      if (active) {
        _treenode_pool.update(frame, idx);
      } else {
        _treenode_pool.clear(idx);
      }
    } else if (active) {
      _treenode_pool.init(frame, id);
    }
  }
};
