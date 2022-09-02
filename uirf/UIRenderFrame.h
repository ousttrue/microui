#pragma once
#include <stdint.h>

struct UICommandRange {
  uint32_t head;
  uint32_t tail;
};

struct UIRenderFrame {
  const struct UICommandRange *command_groups;
  uint32_t command_group_count;
  const uint8_t *command_buffer;

#ifdef __cplusplus
  const UICommandRange *begin() const { return command_groups; }
  const UICommandRange *end() const {
    return command_groups + command_group_count;
  }
#endif
};
