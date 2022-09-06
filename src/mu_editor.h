#pragma once
#include "mu_types.h"
const auto MU_MAX_FMT = 127;
const auto MU_REAL_FMT = "%.3g";

class Editor {
  char number_edit_buf[MU_MAX_FMT] = {0};
  mu_Id number_edit = 0;

public:
  mu_Id id() const { return number_edit; }
  void set_value(mu_Id id, float value) {
    number_edit = id;
    sprintf(number_edit_buf, MU_REAL_FMT, value);
  }

  MU_RES textbox(mu_Context *ctx, const UIRect &r) {
    return mu_textbox_raw(ctx, number_edit_buf, sizeof(number_edit_buf),
                          number_edit, r, MU_OPT::MU_OPT_NONE);
  }

  float commit() {
    number_edit = 0;
    return strtod(number_edit_buf, nullptr);
  }
};
