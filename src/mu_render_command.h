#pragma once
#include <stdint.h>
#include "mu_color.h"
#include "mu_vec2.h"
#include "mu_rect.h"

struct mu_WindowRange {
  uint32_t head = 0;
  uint32_t tail = 0;
};

struct mu_RenderCommand {
  const mu_WindowRange *window_range_list;
  uint32_t window_count;
  const uint8_t *command_buffer;

#ifdef __cplusplus
  const mu_WindowRange *begin() const { return window_range_list; }
  const mu_WindowRange *end() const { return window_range_list + window_count; }
#endif
};
