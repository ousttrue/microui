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

mu_Context *mu_new(text_width_callback text_width_callback,
                   text_height_callback text_height_callback) {
  auto ctx = new mu_Context;
  ctx->style->text_width_callback = text_width_callback;
  ctx->style->text_height_callback = text_height_callback;
  return ctx;
}
void mu_delete(mu_Context *ctx) { delete ctx; }

void mu_input_mousemove(mu_Context *ctx, int x, int y) {
  ctx->_input.set_mousemove(x, y);
}

void mu_input_mousedown(mu_Context *ctx, int button) {
  ctx->_input.set_mousedown(static_cast<MU_MOUSE>(button));
}

void mu_input_mouseup(mu_Context *ctx, int button) {
  ctx->_input.set_mouseup(static_cast<MU_MOUSE>(button));
}

void mu_input_scroll(mu_Context *ctx, int x, int y) {
  ctx->_input.add_scroll_delta(x, y);
}

void mu_begin(mu_Context *ctx) {
  assert(ctx->style->text_width_callback && ctx->style->text_height_callback);
  ctx->_command_stack.begin();
  ctx->scroll_target = nullptr;
  ctx->_container.begin();
  ctx->_input.begin();
  ctx->frame++;
}

void mu_end(mu_Context *ctx, UIRenderFrame *command) {
  // check stacks
  ctx->_command_stack.end();
  ctx->_hash.end();
  assert(ctx->layout_stack.size() == 0);

  // handle scroll input
  auto mouse_pressed = ctx->_input.mouse_pressed();

  if (ctx->scroll_target) {
    ctx->scroll_target->scroll += ctx->_input.scroll_delta();
  }

  // reset input state
  ctx->_input.end();

  ctx->_container.end(mouse_pressed, command);
  command->command_buffer = (const uint8_t *)ctx->_command_stack.get(0);
}

mu_Id mu_get_id(mu_Context *ctx, const void *data, int size) {
  return ctx->_hash.create(data, size);
}

void mu_push_id(mu_Context *ctx, const void *data, int size) {
  ctx->_hash.create_push(data, size);
}

void mu_pop_id(mu_Context *ctx) { ctx->_hash.pop(); }
static void pop_container(mu_Context *ctx) {
  mu_Layout *layout = &ctx->layout_stack.back();
  mu_Container *cnt = ctx->_container.current_container();
  cnt->content_size.x = layout->max.x - layout->body.x;
  cnt->content_size.y = layout->max.y - layout->body.y;
  ctx->_container.pop();
  ctx->layout_stack.pop();
  ctx->_hash.pop();
}

mu_Container *mu_get_current_container(mu_Context *ctx) {
  return ctx->_container.current_container();
}

mu_Container *mu_get_container(mu_Context *ctx, const char *name) {
  mu_Id id = ctx->_hash.create(name, strlen(name));
  return ctx->_container.get_container(id, MU_OPT::MU_OPT_NONE, ctx->frame);
}

/*============================================================================
** layout
**============================================================================*/

enum { RELATIVE = 1, ABSOLUTE = 2 };

void mu_layout_begin_column(mu_Context *ctx) {
  ctx->layout_stack.push(mu_Layout(mu_layout_next(ctx), UIVec2(0, 0)));
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

void mu_layout_set_next(mu_Context *ctx, UIRect r, int relative) {
  mu_Layout *layout = &ctx->layout_stack.back();
  layout->next = r;
  layout->next_type = relative ? RELATIVE : ABSOLUTE;
}

UIRect mu_layout_next(mu_Context *ctx) {
  mu_Layout *layout = &ctx->layout_stack.back();
  mu_Style *style = ctx->style;
  UIRect res;

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
void mu_text(mu_Context *ctx, const char *text) {
  const char *start, *end, *p = text;
  int width = -1;
  UIColor32 color = ctx->style->colors[MU_STYLE_TEXT];
  mu_layout_begin_column(ctx);
  ctx->layout_stack.back().row(1, &width, ctx->style->text_height());
  do {
    UIRect r = mu_layout_next(ctx);
    int w = 0;
    start = end = p;
    do {
      const char *word = p;
      while (*p && *p != ' ' && *p != '\n') {
        p++;
      }
      w += ctx->style->text_width(word, p - word);
      if (w > r.w && end != start) {
        break;
      }
      w += ctx->style->text_width(p, 1);
      end = p++;
    } while (*end && *end != '\n');
    ctx->_command_stack.draw_text(start, end - start, UIVec2(r.x, r.y),
                                  ctx->style, color);
    p = end + 1;
  } while (*end);
  mu_layout_end_column(ctx);
}

void mu_label(mu_Context *ctx, const char *text) {
  ctx->_command_stack.draw_control_text(text, mu_layout_next(ctx), ctx->style,
                                        MU_STYLE_TEXT, MU_OPT::MU_OPT_NONE);
}

MU_RES mu_button_ex(mu_Context *ctx, const char *label, int icon, MU_OPT opt) {
  auto res = MU_RES::MU_RES_NONE;
  mu_Id id = label ? ctx->_hash.create(label, strlen(label))
                   : ctx->_hash.create(&icon, sizeof(icon));
  UIRect r = mu_layout_next(ctx);
  auto mouseover = ctx->mouse_over(r);
  ctx->_input.update_focus_hover(id, r, opt, mouseover);
  // handle click
  if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT &&
      ctx->_input.has_focus(id)) {
    res = res | MU_RES_SUBMIT;
  }
  // draw
  ctx->_command_stack.draw_control_frame(id, r, ctx->style, MU_STYLE_BUTTON,
                                         opt, ctx->_input.get_focus_state(id));
  if (label) {
    ctx->_command_stack.draw_control_text(label, r, ctx->style, MU_STYLE_TEXT,
                                          opt);
  }
  if (icon) {
    ctx->_command_stack.draw_icon(ctx, icon, r,
                                  ctx->style->colors[MU_STYLE_TEXT]);
  }
  return res;
}

MU_RES mu_checkbox(mu_Context *ctx, const char *label, int *state) {
  mu_Id id = ctx->_hash.create(&state, sizeof(state));
  UIRect r = mu_layout_next(ctx);
  UIRect box = UIRect(r.x, r.y, r.h, r.h);
  auto mouseover = ctx->mouse_over(r);
  ctx->_input.update_focus_hover(id, r, MU_OPT::MU_OPT_NONE, mouseover);
  // handle click
  auto res = MU_RES::MU_RES_NONE;
  if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT &&
      ctx->_input.has_focus(id)) {
    res = res | MU_RES_CHANGE;
    *state = !*state;
  }
  // draw
  ctx->_command_stack.draw_control_frame(id, box, ctx->style, MU_STYLE_BASE, MU_OPT::MU_OPT_NONE, ctx->_input.get_focus_state(id));
  if (*state) {
    ctx->_command_stack.draw_icon(ctx, MU_ICON_CHECK, box,
                                  ctx->style->colors[MU_STYLE_TEXT]);
  }
  r = UIRect(r.x + box.w, r.y, r.w - box.w, r.h);
  ctx->_command_stack.draw_control_text(label, r, ctx->style, MU_STYLE_TEXT,
                                        MU_OPT::MU_OPT_NONE);
  return res;
}

MU_RES mu_textbox_raw(mu_Context *ctx, char *buf, int bufsz, mu_Id id, UIRect r,
                      MU_OPT opt) {
  auto res = MU_RES::MU_RES_NONE;
  auto mouseover = ctx->mouse_over(r);
  ctx->_input.update_focus_hover(id, r, opt | MU_OPT_HOLDFOCUS, mouseover);

  if (ctx->_input.has_focus(id)) {
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
      ctx->_input.set_focus(0);
      res = res | MU_RES_SUBMIT;
    }
  }

  // draw
  ctx->_command_stack.draw_control_frame(id, r, ctx->style, MU_STYLE_BASE, opt, ctx->_input.get_focus_state(id));
  if (ctx->_input.has_focus(id)) {
    UIColor32 color = ctx->style->colors[MU_STYLE_TEXT];
    int textw = ctx->style->text_width(buf, -1);
    int texth = ctx->style->text_height();
    int ofx = r.w - ctx->style->padding - textw - 1;
    int textx = r.x + mu_min(ofx, ctx->style->padding);
    int texty = r.y + (r.h - texth) / 2;
    ctx->_command_stack.push_clip(r);
    ctx->_command_stack.draw_text(buf, -1, UIVec2(textx, texty), ctx->style,
                                  color);
    ctx->_command_stack.draw_rect(UIRect(textx + textw, texty, 1, texth),
                                  color);
    ctx->_command_stack.pop_clip();
  } else {
    ctx->_command_stack.draw_control_text(buf, r, ctx->style, MU_STYLE_TEXT,
                                          opt);
  }

  return res;
}

static bool number_textbox(mu_Context *ctx, mu_Real *value, UIRect r,
                           mu_Id id) {
  if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT &&
      ctx->_input.key_down() & MU_KEY_SHIFT && ctx->_input.has_hover(id)) {
    ctx->number_edit = id;
    sprintf(ctx->number_edit_buf, MU_REAL_FMT, *value);
  }
  if (ctx->number_edit == id) {
    int res =
        mu_textbox_raw(ctx, ctx->number_edit_buf, sizeof(ctx->number_edit_buf),
                       id, r, MU_OPT::MU_OPT_NONE);
    if (res & MU_RES_SUBMIT || !ctx->_input.has_focus(id)) {
      *value = strtod(ctx->number_edit_buf, nullptr);
      ctx->number_edit = 0;
    } else {
      return 1;
    }
  }
  return 0;
}

MU_RES mu_textbox_ex(mu_Context *ctx, char *buf, int bufsz, MU_OPT opt) {
  mu_Id id = ctx->_hash.create(&buf, sizeof(buf));
  UIRect r = mu_layout_next(ctx);
  return mu_textbox_raw(ctx, buf, bufsz, id, r, opt);
}

MU_RES mu_slider_ex(mu_Context *ctx, mu_Real *value, mu_Real low, mu_Real high,
                    mu_Real step, const char *fmt, MU_OPT opt) {
  char buf[MU_MAX_FMT + 1];
  UIRect thumb;
  int x, w = 0;
  mu_Real last = *value, v = last;
  mu_Id id = ctx->_hash.create(&value, sizeof(value));
  UIRect base = mu_layout_next(ctx);

  // handle text input mode
  auto res = MU_RES_NONE;
  if (number_textbox(ctx, &v, base, id)) {
    return res;
  }

  // handle normal mode
  auto mouseover = ctx->mouse_over(base);
  ctx->_input.update_focus_hover(id, base, opt, mouseover);

  // handle input
  if (ctx->_input.has_focus(id) &&
      (ctx->_input.mouse_down() | ctx->_input.mouse_pressed()) ==
          MU_MOUSE_LEFT) {
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
  ctx->_command_stack.draw_control_frame(id, base, ctx->style, MU_STYLE_BASE, opt, ctx->_input.get_focus_state(id));
  // draw thumb
  w = ctx->style->thumb_size;
  x = (v - low) * (base.w - w) / (high - low);
  thumb = UIRect(base.x + x, base.y, w, base.h);
  ctx->_command_stack.draw_control_frame(id, thumb, ctx->style, MU_STYLE_BUTTON, opt, ctx->_input.get_focus_state(id));
  // draw text
  sprintf(buf, fmt, v);
  ctx->_command_stack.draw_control_text(buf, base, ctx->style, MU_STYLE_TEXT,
                                        opt);

  return res;
}

MU_RES mu_number_ex(mu_Context *ctx, mu_Real *value, mu_Real step,
                    const char *fmt, MU_OPT opt) {
  char buf[MU_MAX_FMT + 1];
  auto res = MU_RES::MU_RES_NONE;
  mu_Id id = ctx->_hash.create(&value, sizeof(value));
  UIRect base = mu_layout_next(ctx);
  mu_Real last = *value;

  // handle text input mode
  if (number_textbox(ctx, value, base, id)) {
    return res;
  }

  // handle normal mode
  auto mouseover = ctx->mouse_over(base);
  ctx->_input.update_focus_hover(id, base, opt, mouseover);

  // handle input
  if (ctx->_input.has_focus(id) && ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
    *value += ctx->_input.mouse_delta().x * step;
  }
  // set flag if value changed
  if (*value != last) {
    res = res | MU_RES_CHANGE;
  }

  // draw base
  ctx->_command_stack.draw_control_frame(id, base, ctx->style, MU_STYLE_BASE, opt, ctx->_input.get_focus_state(id));
  // draw text
  sprintf(buf, fmt, *value);
  ctx->_command_stack.draw_control_text(buf, base, ctx->style, MU_STYLE_TEXT,
                                        opt);

  return res;
}

static MU_RES header(mu_Context *ctx, const char *label, int istreenode,
                     MU_OPT opt) {
  mu_Id id = ctx->_hash.create(label, strlen(label));
  int idx = ctx->treenode_pool.get_index(id);
  int width = -1;
  ctx->layout_stack.back().row(1, &width, 0);

  auto active = (idx >= 0);
  auto expanded = (opt & MU_OPT_EXPANDED) ? !active : active;
  auto r = mu_layout_next(ctx);
  auto mouseover = ctx->mouse_over(r);
  ctx->_input.update_focus_hover(id, r, MU_OPT::MU_OPT_NONE, mouseover);

  // handle click
  active ^= (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT &&
             ctx->_input.has_focus(id));

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
    if (ctx->_input.has_hover(id)) {
      ctx->_command_stack.draw_frame(r, ctx->style, MU_STYLE_BUTTONHOVER);
    }
  } else {
    ctx->_command_stack.draw_control_frame(id, r, ctx->style, MU_STYLE_BUTTON, MU_OPT::MU_OPT_NONE, ctx->_input.get_focus_state(id));
  }
  ctx->_command_stack.draw_icon(
      ctx, expanded ? MU_ICON_EXPANDED : MU_ICON_COLLAPSED,
      UIRect(r.x, r.y, r.h, r.h), ctx->style->colors[MU_STYLE_TEXT]);
  r.x += r.h - ctx->style->padding;
  r.w -= r.h - ctx->style->padding;
  ctx->_command_stack.draw_control_text(label, r, ctx->style, MU_STYLE_TEXT,
                                        MU_OPT::MU_OPT_NONE);

  return expanded ? MU_RES_ACTIVE : MU_RES_NONE;
}

MU_RES mu_header_ex(mu_Context *ctx, const char *label, MU_OPT opt) {
  return header(ctx, label, 0, opt);
}

MU_RES mu_begin_treenode_ex(mu_Context *ctx, const char *label, MU_OPT opt) {
  auto res = header(ctx, label, 1, opt);
  if (res & MU_RES_ACTIVE) {
    ctx->layout_stack.back().indent += ctx->style->indent;
    ctx->_hash.push_last();
  }
  return res;
}

void mu_end_treenode(mu_Context *ctx) {
  ctx->layout_stack.back().indent -= ctx->style->indent;
  ctx->_hash.pop();
}

static void scrollbar(mu_Context *ctx, mu_Container *cnt, UIRect *b, UIVec2 cs,
                      const char *key) {
  // only add scrollbar if content size is larger than body
  int maxscroll = cs.y - b->h;

  if (maxscroll > 0 && b->h > 0) {
    UIRect base, thumb;
    mu_Id id = ctx->_hash.create(key, 11);

    // get sizing / positioning
    base = *b;
    base.x = b->x + b->w;
    base.w = ctx->style->scrollbar_size;

    // handle input
    auto mouseover = ctx->mouse_over(base);
    ctx->_input.update_focus_hover(id, base, MU_OPT_NONE, mouseover);
    if (ctx->_input.has_focus(id) &&
        ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
      cnt->scroll.y += ctx->_input.mouse_delta().y * cs.y / base.h;
    }
    // clamp scroll to limits
    cnt->scroll.y = mu_clamp(cnt->scroll.y, 0, maxscroll);

    // draw base and thumb
    ctx->_command_stack.draw_frame(base, ctx->style, MU_STYLE_SCROLLBASE);
    thumb = base;
    thumb.h = mu_max(ctx->style->thumb_size, base.h * b->h / cs.y);
    thumb.y += cnt->scroll.y * (base.h - thumb.h) / maxscroll;
    ctx->_command_stack.draw_frame(thumb, ctx->style, MU_STYLE_SCROLLTHUMB);

    // set this as the scroll_target (will get scrolled on mousewheel)
    // if the mouse is over it
    if (ctx->mouse_over(*b)) {
      ctx->scroll_target = cnt;
    }
  } else {
    cnt->scroll.y = 0;
  }
}

static void scrollbars(mu_Context *ctx, mu_Container *cnt, UIRect *body) {
  int sz = ctx->style->scrollbar_size;
  UIVec2 cs = cnt->content_size;
  cs.x += ctx->style->padding * 2;
  cs.y += ctx->style->padding * 2;
  ctx->_command_stack.push_clip(*body);
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
  ctx->_command_stack.pop_clip();
}

static void push_container_body(mu_Context *ctx, mu_Container *cnt, UIRect body,
                                MU_OPT opt) {
  if (~opt & MU_OPT_NOSCROLL) {
    scrollbars(ctx, cnt, &body);
  }
  ctx->layout_stack.push(
      mu_Layout(body.expand(-ctx->style->padding), cnt->scroll));
  cnt->body = body;
}

MU_RES mu_begin_window(mu_Context *ctx, const char *title, UIRect rect,
                       MU_OPT opt) {
  mu_Id id = ctx->_hash.create(title, strlen(title));
  mu_Container *cnt = ctx->_container.get_container(id, opt, ctx->frame);
  if (!cnt || !cnt->open) {
    return MU_RES_NONE;
  }
  ctx->_hash.push(id);

  if (cnt->rect.w == 0) {
    cnt->rect = rect;
  }

  ctx->_container.begin_root_container(cnt, ctx->_command_stack.size(),
                                       ctx->_input.mouse_pos());
  /* clipping is reset here in case a root-container is made within
  ** another root-containers's begin/end block; this prevents the inner
  ** root-container being clipped to the outer */
  ctx->_command_stack.push_unclipped_rect();

  auto body = cnt->rect;
  rect = body;

  // draw frame
  if (~opt & MU_OPT_NOFRAME) {
    ctx->_command_stack.draw_frame(rect, ctx->style, MU_STYLE_WINDOWBG);
  }

  // do title bar
  if (~opt & MU_OPT_NOTITLE) {
    UIRect tr = rect;
    tr.h = ctx->style->title_height;
    ctx->_command_stack.draw_frame(tr, ctx->style, MU_STYLE_TITLEBG);

    // do title text
    if (~opt & MU_OPT_NOTITLE) {
      mu_Id id = ctx->_hash.create("!title", 6);
      auto mouseover = ctx->mouse_over(tr);
      ctx->_input.update_focus_hover(id, tr, opt, mouseover);
      ctx->_command_stack.draw_control_text(title, tr, ctx->style,
                                            MU_STYLE_TITLETEXT, opt);
      if (ctx->_input.has_focus(id) &&
          ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
        cnt->rect.x += ctx->_input.mouse_delta().x;
        cnt->rect.y += ctx->_input.mouse_delta().y;
      }
      body.y += tr.h;
      body.h -= tr.h;
    }

    // do `close` button
    if (~opt & MU_OPT_NOCLOSE) {
      mu_Id id = ctx->_hash.create("!close", 6);
      UIRect r = UIRect(tr.x + tr.w - tr.h, tr.y, tr.h, tr.h);
      tr.w -= r.w;
      ctx->_command_stack.draw_icon(ctx, MU_ICON_CLOSE, r,
                                    ctx->style->colors[MU_STYLE_TITLETEXT]);
      auto mouseover = ctx->mouse_over(r);
      ctx->_input.update_focus_hover(id, r, opt, mouseover);
      if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT &&
          ctx->_input.has_focus(id)) {
        cnt->open = false;
      }
    }
  }

  push_container_body(ctx, cnt, body, opt);

  // do `resize` handle
  if (~opt & MU_OPT_NORESIZE) {
    int sz = ctx->style->title_height;
    mu_Id id = ctx->_hash.create("!resize", 7);
    UIRect r = UIRect(rect.x + rect.w - sz, rect.y + rect.h - sz, sz, sz);
    auto mouseover = ctx->mouse_over(r);
    ctx->_input.update_focus_hover(id, r, opt, mouseover);
    if (ctx->_input.has_focus(id) &&
        ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
      cnt->rect.w = mu_max(96, cnt->rect.w + ctx->_input.mouse_delta().x);
      cnt->rect.h = mu_max(64, cnt->rect.h + ctx->_input.mouse_delta().y);
    }
  }

  // resize to content size
  if (opt & MU_OPT_AUTOSIZE) {
    UIRect r = ctx->layout_stack.back().body;
    cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
    cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
  }

  // close if this is a popup window and elsewhere was clicked
  if (opt & MU_OPT_POPUP && ctx->_input.mouse_pressed()) {
    if (!ctx->_container.is_hover_root(cnt)) {
      cnt->open = false;
    }
  }

  ctx->_command_stack.push_clip(cnt->body);
  return MU_RES_ACTIVE;
}

void mu_end_window(mu_Context *ctx) {
  ctx->_command_stack.pop_clip();
  /* push tail 'goto' jump command and set head 'skip' command. the final steps
  ** on initing these are done in mu_end() */
  mu_Container *cnt = ctx->_container.current_container();
  cnt->range.tail = ctx->_command_stack.size();
  // pop base clip rect and container
  ctx->_command_stack.pop_clip();
  pop_container(ctx);
}

void mu_open_popup(mu_Context *ctx, const char *name) {
  mu_Id id = ctx->_hash.create(name, strlen(name));
  ctx->_container.open_popup(id, ctx->_input.mouse_pos(), ctx->frame);
}

MU_RES mu_begin_popup(mu_Context *ctx, const char *name) {
  auto opt =
      static_cast<MU_OPT>(MU_OPT_POPUP | MU_OPT_AUTOSIZE | MU_OPT_NORESIZE |
                          MU_OPT_NOSCROLL | MU_OPT_NOTITLE | MU_OPT_CLOSED);
  return mu_begin_window(ctx, name, UIRect(0, 0, 0, 0), opt);
}

void mu_end_popup(mu_Context *ctx) { mu_end_window(ctx); }

void mu_begin_panel_ex(mu_Context *ctx, const char *name, MU_OPT opt) {
  auto last_id = ctx->_hash.create_push(name, strlen(name));
  auto cnt = ctx->_container.get_container(last_id, opt, ctx->frame);
  cnt->rect = mu_layout_next(ctx);
  if (~opt & MU_OPT_NOFRAME) {
    ctx->_command_stack.draw_frame(cnt->rect, ctx->style, MU_STYLE_PANELBG);
  }
  ctx->_container.push(cnt);
  push_container_body(ctx, cnt, cnt->rect, opt);
  ctx->_command_stack.push_clip(cnt->body);
}

void mu_end_panel(mu_Context *ctx) {
  ctx->_command_stack.pop_clip();
  pop_container(ctx);
}
