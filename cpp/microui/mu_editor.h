#pragma once
#include "mu_types.h"
const auto MU_MAX_FMT = 127;
const auto MU_REAL_FMT = "%.3g";

class Editor {
  char _number_edit_buf[MU_MAX_FMT] = {0};
  mu_Id _number_edit = 0;

  mu_Id id() const { return _number_edit; }

public:
  void set_value(mu_Id id, float value) {
    _number_edit = id;
    sprintf(_number_edit_buf, MU_REAL_FMT, value);
  }

  char *buffer(mu_Id id, size_t *size) {
    if (id != _number_edit) {
      return nullptr;
    }
    *size = sizeof(_number_edit_buf);
    return _number_edit_buf;
  }

  float commit() {
    _number_edit = 0;
    return strtod(_number_edit_buf, nullptr);
  }
};
