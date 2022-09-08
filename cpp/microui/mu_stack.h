#pragma once
#include <assert.h>

template <typename T, size_t N> class mu_Stack {
  T items[N] = {};
  int idx = 0;

public:
  const T *begin() const { return items; }
  T *begin() { return items; }
  T *end() { return items + idx; }
  int size() const { return idx; }
  const T &get(int i) const { return items[i]; }
  T &get(int i) { return items[i]; }
  T &back() {
    assert(idx > 0);
    return items[idx - 1];
  }
  void grow(int size) {
    assert(idx + size < N);
    idx += size;
  }
  const T &back() const { return items[idx - 1]; }
  void push(const T &val) {
    assert(this->idx < (int)(sizeof(this->items) / sizeof(*this->items)));
    this->items[this->idx] = (val);
    this->idx++; /* incremented after incase `val` uses this value */
  }

  void pop() {
    assert(this->idx > 0);
    this->idx--;
  }

  void clear() { idx = 0; }
};
