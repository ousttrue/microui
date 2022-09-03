#include <Windows.h>

#include <Gl/GL.h>

#include "atlas.h"
#include "renderer.h"
#include <UIRenderFrame.h>
#include <algorithm>
#include <assert.h>
// #include <mu_context.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 16384

static GLfloat tex_buf[BUFFER_SIZE * 8];
static GLfloat vert_buf[BUFFER_SIZE * 8];
static GLubyte color_buf[BUFFER_SIZE * 16];
static GLuint index_buf[BUFFER_SIZE * 6];

static int width = 800;
static int height = 600;
void r_resize(int w, int h) {
  width = w;
  height = h;
}
static int buf_idx;

void r_init(void) {
  /* init gl */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  /* init texture */
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, ATLAS_WIDTH, ATLAS_HEIGHT, 0,
               GL_ALPHA, GL_UNSIGNED_BYTE, atlas_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  assert(glGetError() == 0);
}

void flush(void) {
  if (buf_idx == 0) {
    return;
  }

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0f, width, height, 0.0f, -1.0f, +1.0f);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glTexCoordPointer(2, GL_FLOAT, 0, tex_buf);
  glVertexPointer(2, GL_FLOAT, 0, vert_buf);
  glColorPointer(4, GL_UNSIGNED_BYTE, 0, color_buf);
  glDrawElements(GL_TRIANGLES, buf_idx * 6, GL_UNSIGNED_INT, index_buf);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  buf_idx = 0;
}

static void push_quad(const UIRect &dst, const UIRect &src,
                      const UIColor32 &color) {
  if (buf_idx == BUFFER_SIZE) {
    flush();
  }

  int texvert_idx = buf_idx * 8;
  int color_idx = buf_idx * 16;
  int element_idx = buf_idx * 4;
  int index_idx = buf_idx * 6;
  buf_idx++;

  /* update texture buffer */
  float x = src.x / (float)ATLAS_WIDTH;
  float y = src.y / (float)ATLAS_HEIGHT;
  float w = src.w / (float)ATLAS_WIDTH;
  float h = src.h / (float)ATLAS_HEIGHT;
  tex_buf[texvert_idx + 0] = x;
  tex_buf[texvert_idx + 1] = y;
  tex_buf[texvert_idx + 2] = x + w;
  tex_buf[texvert_idx + 3] = y;
  tex_buf[texvert_idx + 4] = x;
  tex_buf[texvert_idx + 5] = y + h;
  tex_buf[texvert_idx + 6] = x + w;
  tex_buf[texvert_idx + 7] = y + h;

  /* update vertex buffer */
  vert_buf[texvert_idx + 0] = dst.x;
  vert_buf[texvert_idx + 1] = dst.y;
  vert_buf[texvert_idx + 2] = dst.x + dst.w;
  vert_buf[texvert_idx + 3] = dst.y;
  vert_buf[texvert_idx + 4] = dst.x;
  vert_buf[texvert_idx + 5] = dst.y + dst.h;
  vert_buf[texvert_idx + 6] = dst.x + dst.w;
  vert_buf[texvert_idx + 7] = dst.y + dst.h;

  /* update color buffer */
  memcpy(color_buf + color_idx + 0, &color, 4);
  memcpy(color_buf + color_idx + 4, &color, 4);
  memcpy(color_buf + color_idx + 8, &color, 4);
  memcpy(color_buf + color_idx + 12, &color, 4);

  /* update index buffer */
  index_buf[index_idx + 0] = element_idx + 0;
  index_buf[index_idx + 1] = element_idx + 1;
  index_buf[index_idx + 2] = element_idx + 2;
  index_buf[index_idx + 3] = element_idx + 2;
  index_buf[index_idx + 4] = element_idx + 3;
  index_buf[index_idx + 5] = element_idx + 1;
}

void r_draw_rect(UIRect rect, UIColor32 color) {
  push_quad(rect, *((UIRect *)&atlas[ATLAS_WHITE]), color);
}

void r_draw_text(const uint8_t *begin, const uint8_t *end, UIVec2 pos,
                 UIColor32 color) {
  UIRect dst = {pos.x, pos.y, 0, 0};
  for (auto *p = begin; p != end; p++) {
    if ((*p & 0xc0) == 0x80) {
      continue;
    }
    int chr = std::min((uint8_t)*p, (uint8_t)127);
    UIRect src = *((UIRect *)&atlas[ATLAS_FONT + chr]);
    dst.w = src.w;
    dst.h = src.h;
    push_quad(dst, src, color);
    dst.x += dst.w;
  }
}

void r_draw_icon(int id, UIRect rect, UIColor32 color) {
  UIRect src = *((UIRect *)&atlas[id]);
  int x = rect.x + (rect.w - src.w) / 2;
  int y = rect.y + (rect.h - src.h) / 2;
  push_quad(UIRect(x, y, src.w, src.h), src, color);
}

int r_get_text_width(const char *text, int len) {
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

int r_get_text_height(void) { return 18; }

void r_set_clip_rect(UIRect rect) {
  flush();
  glScissor(rect.x, height - (rect.y + rect.h), rect.w, rect.h);
}

void r_clear(UIColor32 clr) {
  flush();
  glClearColor(clr.r / 255., clr.g / 255., clr.b / 255., clr.a / 255.);
  glClear(GL_COLOR_BUFFER_BIT);
}

void render(int width, int height, float bg[4], const UIRenderFrame *command) {
  r_resize(width, height);

  // /* render */
  glViewport(0, 0, width, height);
  glScissor(0, 0, width, height);
  r_clear(UIColor32(bg[0], bg[1], bg[2], 255));

  auto end = command->end();
  for (auto it = command->begin(); it != end; ++it) {
    auto tail = command->command_buffer + it->tail;
    UICommand *cmd = nullptr;
    for (auto p = command->command_buffer + it->head; p != tail;
         p = p + cmd->size()) {
      cmd = (UICommand *)p;
      switch (cmd->command) {
      case UI_COMMAND_TEXT:
        r_draw_text(cmd->text()->begin(), cmd->text()->end(), cmd->text()->pos,
                    cmd->text()->color);
        break;
      case UI_COMMAND_RECT:
        r_draw_rect(cmd->rect()->rect, cmd->rect()->color);
        break;
      case UI_COMMAND_ICON:
        r_draw_icon(cmd->icon()->id, cmd->icon()->rect, cmd->icon()->color);
        break;
      case UI_COMMAND_CLIP:
        r_set_clip_rect(cmd->clip()->rect);
        break;
      }
    }
  }
  flush();
}
