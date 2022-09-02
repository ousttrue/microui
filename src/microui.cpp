/*
** Copyright (c) 2020 rxi
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
*/

#include "microui.h"
#include "mu_context.h"
#include "mu_layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>

inline MU_RES operator|(MU_RES L, MU_RES R) {
  return static_cast<MU_RES>(
      static_cast<std::underlying_type<MU_RES>::type>(L) |
      static_cast<std::underlying_type<MU_RES>::type>(R));
}
inline MU_RES operator&(MU_RES L, MU_RES R) {
  return static_cast<MU_RES>(
      static_cast<std::underlying_type<MU_RES>::type>(L) &
      static_cast<std::underlying_type<MU_RES>::type>(R));
}
inline MU_OPT operator|(MU_OPT L, MU_OPT R) {
  return static_cast<MU_OPT>(
      static_cast<std::underlying_type<MU_OPT>::type>(L) |
      static_cast<std::underlying_type<MU_OPT>::type>(R));
}
inline MU_OPT operator&(MU_OPT L, MU_OPT R) {
  return static_cast<MU_OPT>(
      static_cast<std::underlying_type<MU_OPT>::type>(L) &
      static_cast<std::underlying_type<MU_OPT>::type>(R));
}

mu_Context *mu_new(text_width_callback text_width,
                   text_height_callback text_height) {
  auto ctx = new mu_Context;
  ctx->text_width = (text_width_callback)text_width;
  ctx->text_height = (text_height_callback)text_height;
  return ctx;
}
void mu_delete(mu_Context *ctx) { delete ctx; }

void mu_input_mousemove(mu_Context *ctx, int x, int y) {
  ctx->_input.mousemove(x, y);
}

void mu_input_mousedown(mu_Context *ctx, int button) {
  ctx->_input.mousedown(static_cast<MU_MOUSE>(button));
}

void mu_input_mouseup(mu_Context *ctx, int button) {
  ctx->_input.mouseup(static_cast<MU_MOUSE>(button));
}

void mu_input_scroll(mu_Context *ctx, int x, int y) {
  ctx->_input.scroll(x, y);
}

void mu_begin(mu_Context *ctx) {
  assert(ctx->text_width && ctx->text_height);
  ctx->_command_stack.begin_frame();
  ctx->root_list.clear();
  ctx->scroll_target = nullptr;
  ctx->hover_root = ctx->next_hover_root;
  ctx->next_hover_root = nullptr;
  ctx->_input.begin();
  ctx->frame++;
}

static int compare_zindex(const void *a, const void *b) {
  return (*(mu_Container **)a)->zindex - (*(mu_Container **)b)->zindex;
}

void mu_end(mu_Context *ctx, UIRenderFrame *command) {
  // check stacks
  assert(ctx->container_stack.size() == 0);
  assert(ctx->clip_stack.size() == 0);
  assert(ctx->id_stack.size() == 0);
  assert(ctx->layout_stack.size() == 0);

  // handle scroll input
  ctx->end_input();

  // sort root containers by zindex
  auto n = ctx->root_list.size();
  qsort(ctx->root_list.begin(), n, sizeof(mu_Container *), compare_zindex);

  auto end = ctx->root_list.end();
  auto p = &ctx->root_window_ranges[0];
  for (auto it = ctx->root_list.begin(); it != end; ++it, ++p) {
    *p = (*it)->range;
  }

  command->command_groups = &ctx->root_window_ranges[0];
  command->command_group_count = ctx->root_list.size();
  command->command_buffer = (const uint8_t *)ctx->_command_stack.get(0);
}

// 32bit fnv-1a hash
#define HASH_INITIAL 2166136261

static void hash(mu_Id *hash, const void *data, int size) {
  auto p = (const unsigned char *)data;
  while (size--) {
    *hash = (*hash ^ *p++) * 16777619;
  }
}

mu_Id mu_get_id(mu_Context *ctx, const void *data, int size) {
  int idx = ctx->id_stack.size();
  mu_Id res = (idx > 0) ? ctx->id_stack.back() : HASH_INITIAL;
  hash(&res, data, size);
  ctx->last_id = res;
  return res;
}

void mu_push_id(mu_Context *ctx, const void *data, int size) {
  ctx->id_stack.push(mu_get_id(ctx, data, size));
}

void mu_pop_id(mu_Context *ctx) { ctx->id_stack.pop(); }

static void pop_container(mu_Context *ctx) {
  mu_Container *cnt = mu_get_current_container(ctx);
  mu_Layout *layout = &ctx->layout_stack.back();
  cnt->content_size.x = layout->max.x - layout->body.x;
  cnt->content_size.y = layout->max.y - layout->body.y;
  // pop container, layout and id
  ctx->container_stack.pop();
  ctx->layout_stack.pop();
  mu_pop_id(ctx);
}

mu_Container *mu_get_current_container(mu_Context *ctx) {
  return ctx->container_stack.back();
}

static mu_Container *get_container(mu_Context *ctx, mu_Id id, MU_OPT opt) {
  // try to get existing container from pool
  {
    int idx = ctx->container_pool.get_index(id);
    if (idx >= 0) {
      if (ctx->containers[idx].open || ~opt & MU_OPT_CLOSED) {
        ctx->container_pool.update(ctx->frame, idx);
      }
      return &ctx->containers[idx];
    }
  }

  if (opt & MU_OPT_CLOSED) {
    return nullptr;
  }

  // container not found in pool: init new container
  auto idx = ctx->container_pool.init(ctx->frame, id);
  auto cnt = &ctx->containers[idx];
  cnt->init();
  ctx->bring_to_front(cnt);
  return cnt;
}

mu_Container *mu_get_container(mu_Context *ctx, const char *name) {
  mu_Id id = mu_get_id(ctx, name, strlen(name));
  return get_container(ctx, id, MU_OPT::MU_OPT_NONE);
}

/*============================================================================
** commandlist
**============================================================================*/

void mu_draw_text(mu_Context *ctx, mu_Font font, const char *str, int len,
                  mu_Vec2 pos, mu_Color color) {
  mu_Rect rect = mu_Rect(pos.x, pos.y, ctx->text_width(font, str, len),
                         ctx->text_height(font));
  auto clipped = ctx->check_clip(rect);
  if (clipped == MU_CLIP::ALL) {
    return;
  }
  if (clipped == MU_CLIP::PART) {
    ctx->_command_stack.set_clip(ctx->clip_stack.back());
  }
  // add command
  ctx->_command_stack.push_text(str, len, pos, color, font);
  // reset clipping if it was set
  if (clipped != MU_CLIP::NONE) {
    ctx->_command_stack.set_clip(mu_Rect::unclipped_rect);
  }
}

void mu_draw_icon(mu_Context *ctx, int id, mu_Rect rect, mu_Color color) {
  // do clip command if the rect isn't fully contained within the cliprect
  auto clipped = ctx->check_clip(rect);
  if (clipped == MU_CLIP::ALL) {
    return;
  }
  if (clipped == MU_CLIP::PART) {
    ctx->_command_stack.set_clip(ctx->clip_stack.back());
  }
  // do icon command
  ctx->_command_stack.push_icon(id, rect, color);
  // reset clipping if it was set
  if (clipped != MU_CLIP::NONE) {
    ctx->_command_stack.set_clip(mu_Rect::unclipped_rect);
  }
}

/*============================================================================
** layout
**============================================================================*/

enum { RELATIVE = 1, ABSOLUTE = 2 };

void mu_layout_begin_column(mu_Context *ctx) {
  ctx->layout_stack.push(mu_Layout(mu_layout_next(ctx), mu_Vec2(0, 0)));
}

void mu_layout_end_column(mu_Context *ctx) {
  auto b = &ctx->layout_stack.back();
  ctx->layout_stack.pop();
  // inherit position/next_row/max from child layout if they are greater
  auto a = &ctx->layout_stack.back();
  a->position.x = mu_max(a->position.x, b->position.x + b->body.x - a->body.x);
  a->next_row = mu_max(a->next_row, b->next_row + b->body.y - a->body.y);
  a->max.x = mu_max(a->max.x, b->max.x);
  a->max.y = mu_max(a->max.y, b->max.y);
}

void mu_layout_width(mu_Context *ctx, int width) {
  ctx->layout_stack.back().size.x = width;
}

void mu_layout_height(mu_Context *ctx, int height) {
  ctx->layout_stack.back().size.y = height;
}

void mu_layout_set_next(mu_Context *ctx, mu_Rect r, int relative) {
  mu_Layout *layout = &ctx->layout_stack.back();
  layout->next = r;
  layout->next_type = relative ? RELATIVE : ABSOLUTE;
}

mu_Rect mu_layout_next(mu_Context *ctx) {
  mu_Layout *layout = &ctx->layout_stack.back();
  mu_Style *style = ctx->style;
  mu_Rect res;

  if (layout->next_type) {
    // handle rect set by `mu_layout_set_next`
    int type = layout->next_type;
    layout->next_type = 0;
    res = layout->next;
    if (type == ABSOLUTE) {
      return (ctx->last_rect = res);
    }

  } else {
    // handle next row
    if (layout->item_index == layout->items) {
      ctx->layout_stack.back().row(layout->items, nullptr, layout->size.y);
    }

    // position
    res.x = layout->position.x;
    res.y = layout->position.y;

    // size
    res.w =
        layout->items > 0 ? layout->widths[layout->item_index] : layout->size.x;
    res.h = layout->size.y;
    if (res.w == 0) {
      res.w = style->size.x + style->padding * 2;
    }
    if (res.h == 0) {
      res.h = style->size.y + style->padding * 2;
    }
    if (res.w < 0) {
      res.w += layout->body.w - res.x + 1;
    }
    if (res.h < 0) {
      res.h += layout->body.h - res.y + 1;
    }

    layout->item_index++;
  }

  // update position
  layout->position.x += res.w + style->spacing;
  layout->next_row = mu_max(layout->next_row, res.y + res.h + style->spacing);

  // apply body offset
  res.x += layout->body.x;
  res.y += layout->body.y;

  // update max position
  layout->max.x = mu_max(layout->max.x, res.x + res.w);
  layout->max.y = mu_max(layout->max.y, res.y + res.h);

  return (ctx->last_rect = res);
}

/*============================================================================
** controls
**============================================================================*/

static int in_hover_root(mu_Context *ctx) {
  int i = ctx->container_stack.size();
  while (i--) {
    if (ctx->container_stack.get(i) == ctx->hover_root) {
      return 1;
    }
    /* only root containers have their `head` field set; stop searching if we've
    ** reached the current root container */
    if (ctx->container_stack.get(i)->range.head) {
      break;
    }
  }
  return 0;
}

void mu_draw_control_frame(mu_Context *ctx, mu_Id id, mu_Rect rect, int colorid,
                           MU_OPT opt) {
  if (opt & MU_OPT_NOFRAME) {
    return;
  }
  colorid += ctx->has_focus(id) ? 2 : (ctx->hover == id) ? 1 : 0;
  ctx->draw_frame(ctx, rect, colorid);
}

void mu_draw_control_text(mu_Context *ctx, const char *str, mu_Rect rect,
                          int colorid, MU_OPT opt) {
  mu_Font font = ctx->style->font;
  int tw = ctx->text_width(font, str, -1);
  ctx->push_clip_rect(rect);
  mu_Vec2 pos;
  pos.y = rect.y + (rect.h - ctx->text_height(font)) / 2;
  if (opt & MU_OPT_ALIGNCENTER) {
    pos.x = rect.x + (rect.w - tw) / 2;
  } else if (opt & MU_OPT_ALIGNRIGHT) {
    pos.x = rect.x + rect.w - tw - ctx->style->padding;
  } else {
    pos.x = rect.x + ctx->style->padding;
  }
  mu_draw_text(ctx, font, str, -1, pos, ctx->style->colors[colorid]);
  ctx->pop_clip_rect();
}

int mu_mouse_over(mu_Context *ctx, mu_Rect rect) {
  return rect.overlaps_vec2(ctx->_input.mouse_pos()) &&
         ctx->clip_stack.back().overlaps_vec2(ctx->_input.mouse_pos()) &&
         in_hover_root(ctx);
}

void mu_update_control(mu_Context *ctx, mu_Id id, mu_Rect rect, MU_OPT opt) {
  int mouseover = mu_mouse_over(ctx, rect);
  ctx->update_focus(id);
  if (opt & MU_OPT_NOINTERACT) {
    return;
  }
  if (mouseover && !ctx->_input.mouse_down()) {
    ctx->hover = id;
  }

  if (ctx->has_focus(id)) {
    if (ctx->_input.mouse_pressed() && !mouseover) {
      ctx->set_focus(0);
    }
    if (!ctx->_input.mouse_down() && ~opt & MU_OPT_HOLDFOCUS) {
      ctx->set_focus(0);
    }
  }

  if (ctx->hover == id) {
    if (ctx->_input.mouse_pressed()) {
      ctx->set_focus(id);
    } else if (!mouseover) {
      ctx->hover = 0;
    }
  }
}

void mu_text(mu_Context *ctx, const char *text) {
  const char *start, *end, *p = text;
  int width = -1;
  mu_Font font = ctx->style->font;
  mu_Color color = ctx->style->colors[MU_COLOR_TEXT];
  mu_layout_begin_column(ctx);
  ctx->layout_stack.back().row(1, &width, ctx->text_height(font));
  do {
    mu_Rect r = mu_layout_next(ctx);
    int w = 0;
    start = end = p;
    do {
      const char *word = p;
      while (*p && *p != ' ' && *p != '\n') {
        p++;
      }
      w += ctx->text_width(font, word, p - word);
      if (w > r.w && end != start) {
        break;
      }
      w += ctx->text_width(font, p, 1);
      end = p++;
    } while (*end && *end != '\n');
    mu_draw_text(ctx, font, start, end - start, mu_Vec2(r.x, r.y), color);
    p = end + 1;
  } while (*end);
  mu_layout_end_column(ctx);
}

void mu_label(mu_Context *ctx, const char *text) {
  mu_draw_control_text(ctx, text, mu_layout_next(ctx), MU_COLOR_TEXT,
                       MU_OPT::MU_OPT_NONE);
}

MU_RES mu_button_ex(mu_Context *ctx, const char *label, int icon, MU_OPT opt) {
  auto res = MU_RES::MU_RES_NONE;
  mu_Id id = label ? mu_get_id(ctx, label, strlen(label))
                   : mu_get_id(ctx, &icon, sizeof(icon));
  mu_Rect r = mu_layout_next(ctx);
  mu_update_control(ctx, id, r, opt);
  // handle click
  if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT && ctx->has_focus(id)) {
    res = res | MU_RES_SUBMIT;
  }
  // draw
  mu_draw_control_frame(ctx, id, r, MU_COLOR_BUTTON, opt);
  if (label) {
    mu_draw_control_text(ctx, label, r, MU_COLOR_TEXT, opt);
  }
  if (icon) {
    mu_draw_icon(ctx, icon, r, ctx->style->colors[MU_COLOR_TEXT]);
  }
  return res;
}

MU_RES mu_checkbox(mu_Context *ctx, const char *label, int *state) {
  mu_Id id = mu_get_id(ctx, &state, sizeof(state));
  mu_Rect r = mu_layout_next(ctx);
  mu_Rect box = mu_Rect(r.x, r.y, r.h, r.h);
  mu_update_control(ctx, id, r, MU_OPT::MU_OPT_NONE);
  // handle click
  auto res = MU_RES::MU_RES_NONE;
  if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT && ctx->has_focus(id)) {
    res = res | MU_RES_CHANGE;
    *state = !*state;
  }
  // draw
  mu_draw_control_frame(ctx, id, box, MU_COLOR_BASE, MU_OPT::MU_OPT_NONE);
  if (*state) {
    mu_draw_icon(ctx, MU_ICON_CHECK, box, ctx->style->colors[MU_COLOR_TEXT]);
  }
  r = mu_Rect(r.x + box.w, r.y, r.w - box.w, r.h);
  mu_draw_control_text(ctx, label, r, MU_COLOR_TEXT, MU_OPT::MU_OPT_NONE);
  return res;
}

MU_RES mu_textbox_raw(mu_Context *ctx, char *buf, int bufsz, mu_Id id,
                      mu_Rect r, MU_OPT opt) {
  auto res = MU_RES::MU_RES_NONE;
  mu_update_control(ctx, id, r, opt | MU_OPT_HOLDFOCUS);

  if (ctx->has_focus(id)) {
    // handle text input
    int len = strlen(buf);
    int n = mu_min(bufsz - len - 1, (int)strlen(ctx->_input.input_text()));
    if (n > 0) {
      memcpy(buf + len, ctx->_input.input_text(), n);
      len += n;
      buf[len] = '\0';
      res = res | MU_RES_CHANGE;
    }
    // handle backspace
    if (ctx->_input.key_pressed() & MU_KEY_BACKSPACE && len > 0) {
      // skip utf-8 continuation bytes
      while ((buf[--len] & 0xc0) == 0x80 && len > 0)
        ;
      buf[len] = '\0';
      res = res | MU_RES_CHANGE;
    }
    // handle return
    if (ctx->_input.key_pressed() & MU_KEY_RETURN) {
      ctx->set_focus(0);
      res = res | MU_RES_SUBMIT;
    }
  }

  // draw
  mu_draw_control_frame(ctx, id, r, MU_COLOR_BASE, opt);
  if (ctx->has_focus(id)) {
    mu_Color color = ctx->style->colors[MU_COLOR_TEXT];
    mu_Font font = ctx->style->font;
    int textw = ctx->text_width(font, buf, -1);
    int texth = ctx->text_height(font);
    int ofx = r.w - ctx->style->padding - textw - 1;
    int textx = r.x + mu_min(ofx, ctx->style->padding);
    int texty = r.y + (r.h - texth) / 2;
    ctx->push_clip_rect(r);
    mu_draw_text(ctx, font, buf, -1, mu_Vec2(textx, texty), color);
    ctx->draw_rect(mu_Rect(textx + textw, texty, 1, texth), color);
    ctx->pop_clip_rect();
  } else {
    mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, opt);
  }

  return res;
}

static bool number_textbox(mu_Context *ctx, mu_Real *value, mu_Rect r,
                           mu_Id id) {
  if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT &&
      ctx->_input.key_down() & MU_KEY_SHIFT && ctx->hover == id) {
    ctx->number_edit = id;
    sprintf(ctx->number_edit_buf, MU_REAL_FMT, *value);
  }
  if (ctx->number_edit == id) {
    int res =
        mu_textbox_raw(ctx, ctx->number_edit_buf, sizeof(ctx->number_edit_buf),
                       id, r, MU_OPT::MU_OPT_NONE);
    if (res & MU_RES_SUBMIT || !ctx->has_focus(id)) {
      *value = strtod(ctx->number_edit_buf, nullptr);
      ctx->number_edit = 0;
    } else {
      return 1;
    }
  }
  return 0;
}

MU_RES mu_textbox_ex(mu_Context *ctx, char *buf, int bufsz, MU_OPT opt) {
  mu_Id id = mu_get_id(ctx, &buf, sizeof(buf));
  mu_Rect r = mu_layout_next(ctx);
  return mu_textbox_raw(ctx, buf, bufsz, id, r, opt);
}

MU_RES mu_slider_ex(mu_Context *ctx, mu_Real *value, mu_Real low, mu_Real high,
                    mu_Real step, const char *fmt, MU_OPT opt) {
  char buf[MU_MAX_FMT + 1];
  mu_Rect thumb;
  int x, w = 0;
  mu_Real last = *value, v = last;
  mu_Id id = mu_get_id(ctx, &value, sizeof(value));
  mu_Rect base = mu_layout_next(ctx);

  // handle text input mode
  auto res = MU_RES_NONE;
  if (number_textbox(ctx, &v, base, id)) {
    return res;
  }

  // handle normal mode
  mu_update_control(ctx, id, base, opt);

  // handle input
  if (ctx->has_focus(id) && (ctx->_input.mouse_down() |
                             ctx->_input.mouse_pressed()) == MU_MOUSE_LEFT) {
    v = low + (ctx->_input.mouse_pos().x - base.x) * (high - low) / base.w;
    if (step) {
      v = (((v + step / 2) / step)) * step;
    }
  }
  // clamp and store value, update res
  *value = v = mu_clamp(v, low, high);
  if (last != v) {
    res = res | MU_RES_CHANGE;
  }

  // draw base
  mu_draw_control_frame(ctx, id, base, MU_COLOR_BASE, opt);
  // draw thumb
  w = ctx->style->thumb_size;
  x = (v - low) * (base.w - w) / (high - low);
  thumb = mu_Rect(base.x + x, base.y, w, base.h);
  mu_draw_control_frame(ctx, id, thumb, MU_COLOR_BUTTON, opt);
  // draw text
  sprintf(buf, fmt, v);
  mu_draw_control_text(ctx, buf, base, MU_COLOR_TEXT, opt);

  return res;
}

MU_RES mu_number_ex(mu_Context *ctx, mu_Real *value, mu_Real step,
                    const char *fmt, MU_OPT opt) {
  char buf[MU_MAX_FMT + 1];
  auto res = MU_RES::MU_RES_NONE;
  mu_Id id = mu_get_id(ctx, &value, sizeof(value));
  mu_Rect base = mu_layout_next(ctx);
  mu_Real last = *value;

  // handle text input mode
  if (number_textbox(ctx, value, base, id)) {
    return res;
  }

  // handle normal mode
  mu_update_control(ctx, id, base, opt);

  // handle input
  if (ctx->has_focus(id) && ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
    *value += ctx->_input.mouse_delta().x * step;
  }
  // set flag if value changed
  if (*value != last) {
    res = res | MU_RES_CHANGE;
  }

  // draw base
  mu_draw_control_frame(ctx, id, base, MU_COLOR_BASE, opt);
  // draw text
  sprintf(buf, fmt, *value);
  mu_draw_control_text(ctx, buf, base, MU_COLOR_TEXT, opt);

  return res;
}

static MU_RES header(mu_Context *ctx, const char *label, int istreenode,
                     MU_OPT opt) {
  mu_Rect r;
  int active, expanded;
  mu_Id id = mu_get_id(ctx, label, strlen(label));
  int idx = ctx->treenode_pool.get_index(id);
  int width = -1;
  ctx->layout_stack.back().row(1, &width, 0);

  active = (idx >= 0);
  expanded = (opt & MU_OPT_EXPANDED) ? !active : active;
  r = mu_layout_next(ctx);
  mu_update_control(ctx, id, r, MU_OPT::MU_OPT_NONE);

  // handle click
  active ^=
      (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT && ctx->has_focus(id));

  // update pool ref
  if (idx >= 0) {
    if (active) {
      ctx->treenode_pool.update(ctx->frame, idx);
    } else {
      ctx->treenode_pool.clear(idx);
    }
  } else if (active) {
    ctx->treenode_pool.init(ctx->frame, id);
  }

  // draw
  if (istreenode) {
    if (ctx->hover == id) {
      ctx->draw_frame(ctx, r, MU_COLOR_BUTTONHOVER);
    }
  } else {
    mu_draw_control_frame(ctx, id, r, MU_COLOR_BUTTON, MU_OPT::MU_OPT_NONE);
  }
  mu_draw_icon(ctx, expanded ? MU_ICON_EXPANDED : MU_ICON_COLLAPSED,
               mu_Rect(r.x, r.y, r.h, r.h), ctx->style->colors[MU_COLOR_TEXT]);
  r.x += r.h - ctx->style->padding;
  r.w -= r.h - ctx->style->padding;
  mu_draw_control_text(ctx, label, r, MU_COLOR_TEXT, MU_OPT::MU_OPT_NONE);

  return expanded ? MU_RES_ACTIVE : MU_RES_NONE;
}

MU_RES mu_header_ex(mu_Context *ctx, const char *label, MU_OPT opt) {
  return header(ctx, label, 0, opt);
}

MU_RES mu_begin_treenode_ex(mu_Context *ctx, const char *label, MU_OPT opt) {
  auto res = header(ctx, label, 1, opt);
  if (res & MU_RES_ACTIVE) {
    ctx->layout_stack.back().indent += ctx->style->indent;
    ctx->id_stack.push(ctx->last_id);
  }
  return res;
}

void mu_end_treenode(mu_Context *ctx) {
  ctx->layout_stack.back().indent -= ctx->style->indent;
  mu_pop_id(ctx);
}

static void scrollbar(mu_Context *ctx, mu_Container *cnt, mu_Rect *b,
                      mu_Vec2 cs, const char *key) {
  // only add scrollbar if content size is larger than body
  int maxscroll = cs.y - b->h;

  if (maxscroll > 0 && b->h > 0) {
    mu_Rect base, thumb;
    mu_Id id = mu_get_id(ctx, key, 11);

    // get sizing / positioning
    base = *b;
    base.x = b->x + b->w;
    base.w = ctx->style->scrollbar_size;

    // handle input
    mu_update_control(ctx, id, base, MU_OPT_NONE);
    if (ctx->has_focus(id) && ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
      cnt->scroll.y += ctx->_input.mouse_delta().y * cs.y / base.h;
    }
    // clamp scroll to limits
    cnt->scroll.y = mu_clamp(cnt->scroll.y, 0, maxscroll);

    // draw base and thumb
    ctx->draw_frame(ctx, base, MU_COLOR_SCROLLBASE);
    thumb = base;
    thumb.h = mu_max(ctx->style->thumb_size, base.h * b->h / cs.y);
    thumb.y += cnt->scroll.y * (base.h - thumb.h) / maxscroll;
    ctx->draw_frame(ctx, thumb, MU_COLOR_SCROLLTHUMB);

    // set this as the scroll_target (will get scrolled on mousewheel)
    // if the mouse is over it
    if (mu_mouse_over(ctx, *b)) {
      ctx->scroll_target = cnt;
    }
  } else {
    cnt->scroll.y = 0;
  }
}

static void scrollbars(mu_Context *ctx, mu_Container *cnt, mu_Rect *body) {
  int sz = ctx->style->scrollbar_size;
  mu_Vec2 cs = cnt->content_size;
  cs.x += ctx->style->padding * 2;
  cs.y += ctx->style->padding * 2;
  ctx->push_clip_rect(*body);
  // resize body to make room for scrollbars
  if (cs.y > cnt->body.h) {
    body->w -= sz;
  }
  if (cs.x > cnt->body.w) {
    body->h -= sz;
  }
  /* to create a horizontal or vertical scrollbar almost-identical code is
  ** used; only the references to `x|y` `w|h` need to be switched */
  scrollbar(ctx, cnt, body, cs, "!scrollbary"); // x, y, w, h);
  // scrollbar(ctx, cnt, body, cs, "!scrollbarx"); // y, x, h, w);
  ctx->pop_clip_rect();
}

static void push_container_body(mu_Context *ctx, mu_Container *cnt,
                                mu_Rect body, MU_OPT opt) {
  if (~opt & MU_OPT_NOSCROLL) {
    scrollbars(ctx, cnt, &body);
  }
  ctx->layout_stack.push(
      mu_Layout(body.expand(-ctx->style->padding), cnt->scroll));
  cnt->body = body;
}

static void begin_root_container(mu_Context *ctx, mu_Container *cnt) {
  ctx->container_stack.push(cnt);
  // push container to roots list and push head command
  ctx->root_list.push(cnt);
  cnt->range.head = ctx->_command_stack.size();
  /* set as hover root if the mouse is overlapping this container and it has a
  ** higher zindex than the current hover root */
  if (cnt->rect.overlaps_vec2(ctx->_input.mouse_pos()) &&
      (!ctx->next_hover_root || cnt->zindex > ctx->next_hover_root->zindex)) {
    ctx->next_hover_root = cnt;
  }
  /* clipping is reset here in case a root-container is made within
  ** another root-containers's begin/end block; this prevents the inner
  ** root-container being clipped to the outer */
  ctx->clip_stack.push(mu_Rect::unclipped_rect);
}

static void end_root_container(mu_Context *ctx) {
  /* push tail 'goto' jump command and set head 'skip' command. the final steps
  ** on initing these are done in mu_end() */
  mu_Container *cnt = mu_get_current_container(ctx);
  cnt->range.tail = ctx->_command_stack.size();
  // pop base clip rect and container
  ctx->pop_clip_rect();
  pop_container(ctx);
}

MU_RES mu_begin_window(mu_Context *ctx, const char *title, mu_Rect rect,
                       MU_OPT opt) {
  mu_Id id = mu_get_id(ctx, title, strlen(title));
  mu_Container *cnt = get_container(ctx, id, opt);
  if (!cnt || !cnt->open) {
    return MU_RES_NONE;
  }
  ctx->id_stack.push(id);

  if (cnt->rect.w == 0) {
    cnt->rect = rect;
  }
  begin_root_container(ctx, cnt);
  auto body = cnt->rect;
  rect = body;

  // draw frame
  if (~opt & MU_OPT_NOFRAME) {
    ctx->draw_frame(ctx, rect, MU_COLOR_WINDOWBG);
  }

  // do title bar
  if (~opt & MU_OPT_NOTITLE) {
    mu_Rect tr = rect;
    tr.h = ctx->style->title_height;
    ctx->draw_frame(ctx, tr, MU_COLOR_TITLEBG);

    // do title text
    if (~opt & MU_OPT_NOTITLE) {
      mu_Id id = mu_get_id(ctx, "!title", 6);
      mu_update_control(ctx, id, tr, opt);
      mu_draw_control_text(ctx, title, tr, MU_COLOR_TITLETEXT, opt);
      if (ctx->has_focus(id) && ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
        cnt->rect.x += ctx->_input.mouse_delta().x;
        cnt->rect.y += ctx->_input.mouse_delta().y;
      }
      body.y += tr.h;
      body.h -= tr.h;
    }

    // do `close` button
    if (~opt & MU_OPT_NOCLOSE) {
      mu_Id id = mu_get_id(ctx, "!close", 6);
      mu_Rect r = mu_Rect(tr.x + tr.w - tr.h, tr.y, tr.h, tr.h);
      tr.w -= r.w;
      mu_draw_icon(ctx, MU_ICON_CLOSE, r,
                   ctx->style->colors[MU_COLOR_TITLETEXT]);
      mu_update_control(ctx, id, r, opt);
      if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT && ctx->has_focus(id)) {
        cnt->open = false;
      }
    }
  }

  push_container_body(ctx, cnt, body, opt);

  // do `resize` handle
  if (~opt & MU_OPT_NORESIZE) {
    int sz = ctx->style->title_height;
    mu_Id id = mu_get_id(ctx, "!resize", 7);
    mu_Rect r = mu_Rect(rect.x + rect.w - sz, rect.y + rect.h - sz, sz, sz);
    mu_update_control(ctx, id, r, opt);
    if (ctx->has_focus(id) && ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
      cnt->rect.w = mu_max(96, cnt->rect.w + ctx->_input.mouse_delta().x);
      cnt->rect.h = mu_max(64, cnt->rect.h + ctx->_input.mouse_delta().y);
    }
  }

  // resize to content size
  if (opt & MU_OPT_AUTOSIZE) {
    mu_Rect r = ctx->layout_stack.back().body;
    cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
    cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
  }

  // close if this is a popup window and elsewhere was clicked
  if (opt & MU_OPT_POPUP && ctx->_input.mouse_pressed() &&
      ctx->hover_root != cnt) {
    cnt->open = 0;
  }

  ctx->push_clip_rect(cnt->body);
  return MU_RES_ACTIVE;
}

void mu_end_window(mu_Context *ctx) {
  ctx->pop_clip_rect();
  end_root_container(ctx);
}

void mu_open_popup(mu_Context *ctx, const char *name) {
  mu_Container *cnt = mu_get_container(ctx, name);
  // set as hover root so popup isn't closed in begin_window_ex()
  ctx->hover_root = ctx->next_hover_root = cnt;
  // position at mouse cursor, open and bring-to-front
  cnt->rect =
      mu_Rect(ctx->_input.mouse_pos().x, ctx->_input.mouse_pos().y, 1, 1);
  cnt->open = 1;
  ctx->bring_to_front(cnt);
}

MU_RES mu_begin_popup(mu_Context *ctx, const char *name) {
  auto opt =
      static_cast<MU_OPT>(MU_OPT_POPUP | MU_OPT_AUTOSIZE | MU_OPT_NORESIZE |
                          MU_OPT_NOSCROLL | MU_OPT_NOTITLE | MU_OPT_CLOSED);
  return mu_begin_window(ctx, name, mu_Rect(0, 0, 0, 0), opt);
}

void mu_end_popup(mu_Context *ctx) { mu_end_window(ctx); }

void mu_begin_panel_ex(mu_Context *ctx, const char *name, MU_OPT opt) {
  mu_push_id(ctx, name, strlen(name));
  auto cnt = get_container(ctx, ctx->last_id, opt);
  cnt->rect = mu_layout_next(ctx);
  if (~opt & MU_OPT_NOFRAME) {
    ctx->draw_frame(ctx, cnt->rect, MU_COLOR_PANELBG);
  }
  ctx->container_stack.push(cnt);
  push_container_body(ctx, cnt, cnt->rect, opt);
  ctx->push_clip_rect(cnt->body);
}

void mu_end_panel(mu_Context *ctx) {
  ctx->pop_clip_rect();
  pop_container(ctx);
}
