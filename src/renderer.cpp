#include <Windows.h>
#include <Gl/GL.h>
#include "renderer.h"
#include "atlas.h"
#include <assert.h>
#include <mu_context.h>
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

static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
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

void r_draw_rect(mu_Rect rect, mu_Color color) {
  push_quad(rect, *((mu_Rect *)&atlas[ATLAS_WHITE]), color);
}

void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
  mu_Rect dst = {pos.x, pos.y, 0, 0};
  for (const char *p = text; *p; p++) {
    if ((*p & 0xc0) == 0x80) {
      continue;
    }
    int chr = mu_min((unsigned char)*p, 127);
    mu_Rect src = *((mu_Rect *)&atlas[ATLAS_FONT + chr]);
    dst.w = src.w;
    dst.h = src.h;
    push_quad(dst, src, color);
    dst.x += dst.w;
  }
}

void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
  mu_Rect src = *((mu_Rect *)&atlas[id]);
  int x = rect.x + (rect.w - src.w) / 2;
  int y = rect.y + (rect.h - src.h) / 2;
  push_quad(mu_Rect(x, y, src.w, src.h), src, color);
}

int r_get_text_width(const char *text, int len) {
  int res = 0;
  for (const char *p = text; *p && len--; p++) {
    if ((*p & 0xc0) == 0x80) {
      continue;
    }
    int chr = mu_min((unsigned char)*p, 127);
    res += atlas[ATLAS_FONT + chr].w;
  }
  return res;
}

int r_get_text_height(void) { return 18; }

void r_set_clip_rect(mu_Rect rect) {
  flush();
  glScissor(rect.x, height - (rect.y + rect.h), rect.w, rect.h);
}

void r_clear(mu_Color clr) {
  flush();
  glClearColor(clr.r / 255., clr.g / 255., clr.b / 255., clr.a / 255.);
  glClear(GL_COLOR_BUFFER_BIT);
}

static char logbuf[64000];
static int logbuf_updated = 0;
static float bg[3] = {90, 95, 100};

static void write_log(const char *text) {
  if (logbuf[0]) {
    strcat(logbuf, "\n");
  }
  strcat(logbuf, text);
  logbuf_updated = 1;
}

static void test_window(mu_Context *ctx) {
  /* do window */
  if (mu_begin_window(ctx, "Demo Window", mu_Rect(40, 40, 300, 450),
                      MU_OPT_NONE)) {
    auto win = mu_get_current_container(ctx);
    win->rect.w = mu_max(win->rect.w, 240);
    win->rect.h = mu_max(win->rect.h, 300);

    /* window info */
    if (mu_header(ctx, "Window Info")) {
      auto win = mu_get_current_container(ctx);
      char buf[64];
      {
        int widths[] = {54, -1};
        ctx->layout_stack.back().row(2, widths, 0);
      }
      mu_label(ctx, "Position:");
      sprintf(buf, "%d, %d", win->rect.x, win->rect.y);
      mu_label(ctx, buf);
      mu_label(ctx, "Size:");
      sprintf(buf, "%d, %d", win->rect.w, win->rect.h);
      mu_label(ctx, buf);
    }

    /* labels + buttons */
    if (mu_header_ex(ctx, "Test Buttons", MU_OPT_EXPANDED)) {
      {
        int widths[] = {86, -110, -1};
        ctx->layout_stack.back().row(3, widths, 0);
      }
      mu_label(ctx, "Test buttons 1:");
      if (mu_button(ctx, "Button 1")) {
        write_log("Pressed button 1");
      }
      if (mu_button(ctx, "Button 2")) {
        write_log("Pressed button 2");
      }
      mu_label(ctx, "Test buttons 2:");
      if (mu_button(ctx, "Button 3")) {
        write_log("Pressed button 3");
      }
      if (mu_button(ctx, "Popup")) {
        mu_open_popup(ctx, "Test Popup");
      }
      if (mu_begin_popup(ctx, "Test Popup")) {
        mu_button(ctx, "Hello");
        mu_button(ctx, "World");
        mu_end_popup(ctx);
      }
    }

    /* tree */
    if (mu_header_ex(ctx, "Tree and Text", MU_OPT_EXPANDED)) {
      {
        int widths[] = {140, -1};
        ctx->layout_stack.back().row(2, widths, 0);
      }
      mu_layout_begin_column(ctx);
      if (mu_begin_treenode(ctx, "Test 1")) {
        if (mu_begin_treenode(ctx, "Test 1a")) {
          mu_label(ctx, "Hello");
          mu_label(ctx, "world");
          mu_end_treenode(ctx);
        }
        if (mu_begin_treenode(ctx, "Test 1b")) {
          if (mu_button(ctx, "Button 1")) {
            write_log("Pressed button 1");
          }
          if (mu_button(ctx, "Button 2")) {
            write_log("Pressed button 2");
          }
          mu_end_treenode(ctx);
        }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 2")) {
        {
          int widths[] = {54, 54};
          ctx->layout_stack.back().row(2, widths, 0);
        }
        if (mu_button(ctx, "Button 3")) {
          write_log("Pressed button 3");
        }
        if (mu_button(ctx, "Button 4")) {
          write_log("Pressed button 4");
        }
        if (mu_button(ctx, "Button 5")) {
          write_log("Pressed button 5");
        }
        if (mu_button(ctx, "Button 6")) {
          write_log("Pressed button 6");
        }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 3")) {
        static int checks[3] = {1, 0, 1};
        mu_checkbox(ctx, "Checkbox 1", &checks[0]);
        mu_checkbox(ctx, "Checkbox 2", &checks[1]);
        mu_checkbox(ctx, "Checkbox 3", &checks[2]);
        mu_end_treenode(ctx);
      }
      mu_layout_end_column(ctx);

      mu_layout_begin_column(ctx);
      {
        int widths[] = {-1};
        ctx->layout_stack.back().row(1, widths, 0);
      }
      mu_text(
          ctx,
          "Lorem ipsum dolor sit amet, consectetur adipiscing "
          "elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus "
          "ipsum, eu varius magna felis a nulla.");
      mu_layout_end_column(ctx);
    }

    /* background color sliders */
    if (mu_header_ex(ctx, "Background Color", MU_OPT_EXPANDED)) {
      {
        int widths[] = {-78, -1};
        ctx->layout_stack.back().row(2, widths, 74);
      }
      /* sliders */
      mu_layout_begin_column(ctx);
      {
        int widths[] = {46, -1};
        ctx->layout_stack.back().row(2, widths, 0);
      }
      mu_label(ctx, "Red:");
      mu_slider(ctx, &bg[0], 0, 255);
      mu_label(ctx, "Green:");
      mu_slider(ctx, &bg[1], 0, 255);
      mu_label(ctx, "Blue:");
      mu_slider(ctx, &bg[2], 0, 255);
      mu_layout_end_column(ctx);
      /* color preview */
      mu_Rect r = mu_layout_next(ctx);
      ctx->draw_rect(r, mu_Color(bg[0], bg[1], bg[2], 255));
      char buf[32];
      sprintf(buf, "#%02X%02X%02X", (int)bg[0], (int)bg[1], (int)bg[2]);
      mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
    }

    mu_end_window(ctx);
  }
}

static void log_window(mu_Context *ctx) {
  if (mu_begin_window(ctx, "Log Window", mu_Rect(350, 40, 300, 200),
                      MU_OPT_NONE)) {
    /* output text panel */
    {
      int widths[] = {-1};
      ctx->layout_stack.back().row(1, widths, -25);
    }
    mu_begin_panel(ctx, "Log Output");
    auto panel = mu_get_current_container(ctx);
    {
      int widths[] = {-1};
      ctx->layout_stack.back().row(1, widths, -1);
    }
    mu_text(ctx, logbuf);
    mu_end_panel(ctx);
    if (logbuf_updated) {
      panel->scroll.y = panel->content_size.y;
      logbuf_updated = 0;
    }

    /* input textbox + submit button */
    static char buf[128];
    int submitted = 0;
    {
      int widths[] = {-70, -1};
      ctx->layout_stack.back().row(2, widths, 0);
    }
    if (mu_textbox(ctx, buf, sizeof(buf)) & MU_RES_SUBMIT) {
      ctx->set_focus(ctx->last_id);
      submitted = 1;
    }
    if (mu_button(ctx, "Submit")) {
      submitted = 1;
    }
    if (submitted) {
      write_log(buf);
      buf[0] = '\0';
    }

    mu_end_window(ctx);
  }
}

static int uint8_slider(mu_Context *ctx, unsigned char *value, int low,
                        int high) {
  static float tmp;
  mu_push_id(ctx, &value, sizeof(value));
  tmp = *value;
  int res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
  *value = tmp;
  mu_pop_id(ctx);
  return res;
}

static void style_window(mu_Context *ctx) {
  static struct {
    const char *label;
    int idx;
  } colors[] = {{"text:", MU_COLOR_TEXT},
                {"border:", MU_COLOR_BORDER},
                {"windowbg:", MU_COLOR_WINDOWBG},
                {"titlebg:", MU_COLOR_TITLEBG},
                {"titletext:", MU_COLOR_TITLETEXT},
                {"panelbg:", MU_COLOR_PANELBG},
                {"button:", MU_COLOR_BUTTON},
                {"buttonhover:", MU_COLOR_BUTTONHOVER},
                {"buttonfocus:", MU_COLOR_BUTTONFOCUS},
                {"base:", MU_COLOR_BASE},
                {"basehover:", MU_COLOR_BASEHOVER},
                {"basefocus:", MU_COLOR_BASEFOCUS},
                {"scrollbase:", MU_COLOR_SCROLLBASE},
                {"scrollthumb:", MU_COLOR_SCROLLTHUMB},
                {NULL}};

  if (mu_begin_window(ctx, "Style Editor", mu_Rect(350, 250, 300, 240),
                      MU_OPT_NONE)) {
    int sw = mu_get_current_container(ctx)->body.w * 0.14;
    {
      int widths[] = {80, sw, sw, sw, sw, -1};
      ctx->layout_stack.back().row(6, widths, 0);
    }
    for (int i = 0; colors[i].label; i++) {
      mu_label(ctx, colors[i].label);
      uint8_slider(ctx, &ctx->style->colors[i].r, 0, 255);
      uint8_slider(ctx, &ctx->style->colors[i].g, 0, 255);
      uint8_slider(ctx, &ctx->style->colors[i].b, 0, 255);
      uint8_slider(ctx, &ctx->style->colors[i].a, 0, 255);
      ctx->draw_rect(mu_layout_next(ctx), ctx->style->colors[i]);
    }
    mu_end_window(ctx);
  }
}

static void process_frame(mu_Context *ctx) {
  mu_begin(ctx);
  style_window(ctx);
  log_window(ctx);
  test_window(ctx);
  mu_end(ctx);
}

void render(mu_Context *ctx, int width, int height) {
  r_resize(width, height);

  // /* process frame */
  process_frame(ctx);

  // /* render */
  glViewport(0, 0, width, height);
  glScissor(0, 0, width, height);
  r_clear(mu_Color(bg[0], bg[1], bg[2], 255));

  auto end = ctx->root_list.end();
  for (auto it = ctx->root_list.begin(); it != end; ++it) {
    auto tail = ctx->_command_stack.get((*it)->tail);
    mu_Command *cmd = nullptr;
    for (auto p = ctx->_command_stack.get((*it)->head); p != tail;
         p = p + cmd->size) {
      cmd = (mu_Command *)p;
      switch (cmd->type) {
      case MU_COMMAND::TEXT:
        r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color);
        break;
      case MU_COMMAND::RECT:
        r_draw_rect(cmd->rect.rect, cmd->rect.color);
        break;
      case MU_COMMAND::ICON:
        r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
        break;
      case MU_COMMAND::CLIP:
        r_set_clip_rect(cmd->clip.rect);
        break;
      }
    }
  }
  flush();
}
