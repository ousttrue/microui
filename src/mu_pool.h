#pragma once
#include "mu_types.h"

struct mu_PoolItem {
  mu_Id id = 0;
  int last_update = 0;

  void clear() {
    id = 0;
    last_update = 0;
  }
};

template <size_t N> class mu_Pool {
  mu_PoolItem items[N] = {};

public:
  void clear(size_t idx) {
    auto &item = items[idx];
    item.clear();
  }

  int init(int frame, mu_Id id) {
    int n = -1;
    int f = frame;
    for (int i = 0; i < N; i++) {
      if (items[i].last_update < f) {
        f = items[i].last_update;
        n = i;
      }
    }
    assert(n > -1);
    items[n].id = id;
    update(frame, n);
    return n;
  }

  int get_index(mu_Id id) {
    for (int i = 0; i < N; i++) {
      if (items[i].id == id) {
        return i;
      }
    }
    return -1;
  }

  void update(int frame, size_t idx) { items[idx].last_update = frame; }
};
