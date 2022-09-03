#pragma once
#include "mu_stack.h"
#include <assert.h>

using mu_Id = unsigned int;

// 32bit fnv-1a hash
const mu_Id HASH_INITIAL = 2166136261;

const auto MU_IDSTACK_SIZE = 32;
class MuHash {
  mu_Stack<mu_Id, MU_IDSTACK_SIZE> _stack;
  mu_Id _last = 0;

  static void hash(mu_Id *hash, const void *data, int size) {
    auto p = (const unsigned char *)data;
    while (size--) {
      *hash = (*hash ^ *p++) * 16777619;
    }
  }

public:
  void validate_empty() { assert(_stack.size() == 0); }

  mu_Id create(const void *data, int size) {
    int idx = _stack.size();
    mu_Id res = (idx > 0) ? _stack.back() : HASH_INITIAL;
    hash(&res, data, size);
    _last = res;
    return res;
  }

  mu_Id last() const { return _last; }

  mu_Id create_push(const void *data, int size) {
    auto id = create(data, size);
    _stack.push(id);
    return id;
  }

  void push(mu_Id id) { _stack.push(id); }

  void push_last() { _stack.push(_last); }

  void pop() { _stack.pop(); }
};
