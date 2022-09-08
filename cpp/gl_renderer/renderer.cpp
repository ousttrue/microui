#include "renderer.h"
#include "atlas.h"
#include "renderer_impl.h"
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <string.h>

Renderer g_renderer;

void MUI_RENDERER_init(void *p) {
  g_renderer.initialize(p);
  g_renderer.load_atlas(ATLAS_WIDTH, ATLAS_HEIGHT, atlas_texture);
}

int MUI_RENDERER_get_text_width(const char *text, int len) {
  int res = 0;
  for (const char *p = text; *p && len--; p++) {
    if ((*p & 0xc0) == 0x80) {
      continue;
    }
    int chr = std::min((uint8_t)*p, (uint8_t)127);
    res += atlas[ATLAS_FONT + chr].w;
  }
  return res;
}

int MUI_RENDERER_get_text_height(void) { return 18; }

void MUI_RENDERER_render(int width, int height, float bg[4],
                         const UIRenderFrame *command) {
  g_renderer.begin(width, height, UIColor32(bg[0], bg[1], bg[2], 255));

  auto end = command->end();
  for (auto it = command->begin(); it != end; ++it) {
    auto tail = command->command_buffer + it->tail;
    UICommandHeader *cmd = nullptr;
    for (auto p = command->command_buffer + it->head; p != tail;
         p = p + cmd->size()) {
      cmd = (UICommandHeader *)p;
      switch (cmd->command) {
      case UI_COMMAND_TEXT:
        g_renderer.draw_text(cmd->text()->begin(), cmd->text()->end(),
                             cmd->text()->pos, cmd->text()->color);
        break;
      case UI_COMMAND_RECT:
        g_renderer.draw_rect(cmd->rect()->rect, cmd->rect()->color);
        break;
      case UI_COMMAND_ICON:
        g_renderer.draw_icon(cmd->icon()->id, cmd->icon()->rect,
                             cmd->icon()->color);
        break;
      case UI_COMMAND_CLIP:
        g_renderer.set_clip_rect(cmd->clip()->rect);
        break;
      }
    }
  }

  g_renderer.flush();
}
