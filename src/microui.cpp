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
  ctx->_command_drawer.style()->text_width_callback = text_width_callback;
  ctx->_command_drawer.style()->text_height_callback = text_height_callback;
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
  ctx->_command_drawer.begin();
  ctx->_container.begin();
  ctx->_input.begin();
  ctx->frame++;
}

void mu_end(mu_Context *ctx, UIRenderFrame *command) {
  ctx->_hash.end();
  ctx->_layout.end();
  auto mouse_pressed = ctx->_input.end();
  ctx->_container.end(mouse_pressed, command);
  ctx->_command_drawer.end(command);
}

/*============================================================================
** controls
**============================================================================*/
void mu_text(mu_Context *ctx, const char *text) {
  auto style = ctx->_command_drawer.style();
  const char *start, *end, *p = text;
  int width = -1;
  ctx->_layout.begin_column(style);
  ctx->_layout.back().row(1, &width, style->text_height());
  do {
    UIRect r = ctx->_layout.next(style);
    int w = 0;
    start = end = p;
    do {
      const char *word = p;
      while (*p && *p != ' ' && *p != '\n') {
        p++;
      }
      w += style->text_width(word, p - word);
      if (w > r.w && end != start) {
        break;
      }
      w += style->text_width(p, 1);
      end = p++;
    } while (*end && *end != '\n');
    ctx->_command_drawer.draw_text(start, end - start, UIVec2(r.x, r.y),
                                   MU_STYLE_TEXT);
    p = end + 1;
  } while (*end);
  ctx->_layout.end_column();
}

void mu_label(mu_Context *ctx, const char *text) {
  auto style = ctx->_command_drawer.style();
  ctx->_command_drawer.draw_control_text(text, ctx->_layout.next(style),
                                         MU_STYLE_TEXT, MU_OPT::MU_OPT_NONE);
}

MU_RES mu_button_ex(mu_Context *ctx, const char *label, int icon, MU_OPT opt) {
  auto res = MU_RES::MU_RES_NONE;
  mu_Id id = label ? ctx->_hash.create(label, strlen(label))
                   : ctx->_hash.create(&icon, sizeof(icon));
  auto style = ctx->_command_drawer.style();
  UIRect r = ctx->_layout.next(style);
  auto mouseover = ctx->mouse_over(r);
  ctx->_input.update_focus_hover(id, opt, mouseover);
  // handle click
  if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT &&
      ctx->_input.has_focus(id)) {
    res = res | MU_RES_SUBMIT;
  }
  // draw
  ctx->_command_drawer.draw_control_frame(id, r, MU_STYLE_BUTTON, opt,
                                          ctx->_input.get_focus_state(id));
  if (label) {
    ctx->_command_drawer.draw_control_text(label, r, MU_STYLE_TEXT, opt);
  }
  if (icon) {
    ctx->_command_drawer.draw_icon(ctx, icon, r, MU_STYLE_TEXT);
  }
  return res;
}

MU_RES mu_checkbox(mu_Context *ctx, const char *label, int *state) {
  mu_Id id = ctx->_hash.create(&state, sizeof(state));
  auto style = ctx->_command_drawer.style();
  UIRect r = ctx->_layout.next(style);
  UIRect box = UIRect(r.x, r.y, r.h, r.h);
  auto mouseover = ctx->mouse_over(r);
  ctx->_input.update_focus_hover(id, MU_OPT::MU_OPT_NONE, mouseover);
  // handle click
  auto res = MU_RES::MU_RES_NONE;
  if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT &&
      ctx->_input.has_focus(id)) {
    res = res | MU_RES_CHANGE;
    *state = !*state;
  }
  // draw
  ctx->_command_drawer.draw_control_frame(id, box, MU_STYLE_BASE,
                                          MU_OPT::MU_OPT_NONE,
                                          ctx->_input.get_focus_state(id));
  if (*state) {
    ctx->_command_drawer.draw_icon(ctx, MU_ICON_CHECK, box, MU_STYLE_TEXT);
  }
  r = UIRect(r.x + box.w, r.y, r.w - box.w, r.h);
  ctx->_command_drawer.draw_control_text(label, r, MU_STYLE_TEXT,
                                         MU_OPT::MU_OPT_NONE);
  return res;
}

MU_RES mu_textbox_raw(mu_Context *ctx, char *buf, int bufsz, mu_Id id, UIRect r,
                      MU_OPT opt) {
  auto res = MU_RES::MU_RES_NONE;
  auto mouseover = ctx->mouse_over(r);
  ctx->_input.update_focus_hover(id, opt | MU_OPT_HOLDFOCUS, mouseover);

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
  ctx->_command_drawer.draw_control_frame(id, r, MU_STYLE_BASE, opt,
                                          ctx->_input.get_focus_state(id));
  if (ctx->_input.has_focus(id)) {
    auto style = ctx->_command_drawer.style();
    int textw = style->text_width(buf, -1);
    int texth = style->text_height();
    int ofx = r.w - style->padding - textw - 1;
    int textx = r.x + mu_min(ofx, style->padding);
    int texty = r.y + (r.h - texth) / 2;
    ctx->_command_drawer.push_clip(r);
    ctx->_command_drawer.draw_text(buf, -1, UIVec2(textx, texty),
                                   MU_STYLE_TEXT);
    ctx->_command_drawer.draw_rect(UIRect(textx + textw, texty, 1, texth),
                                   MU_STYLE_TEXT);
    ctx->_command_drawer.pop_clip();
  } else {
    ctx->_command_drawer.draw_control_text(buf, r, MU_STYLE_TEXT, opt);
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
  auto style = ctx->_command_drawer.style();
  UIRect r = ctx->_layout.next(style);
  return mu_textbox_raw(ctx, buf, bufsz, id, r, opt);
}

MU_RES mu_slider_ex(mu_Context *ctx, mu_Real *value, mu_Real low, mu_Real high,
                    mu_Real step, const char *fmt, MU_OPT opt) {
  char buf[MU_MAX_FMT + 1];
  UIRect thumb;
  mu_Real last = *value, v = last;
  mu_Id id = ctx->_hash.create(&value, sizeof(value));
  auto style = ctx->_command_drawer.style();
  UIRect base = ctx->_layout.next(style);

  // handle text input mode
  auto res = MU_RES_NONE;
  if (number_textbox(ctx, &v, base, id)) {
    return res;
  }

  // handle normal mode
  auto mouseover = ctx->mouse_over(base);
  ctx->_input.update_focus_hover(id, opt, mouseover);

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
  ctx->_command_drawer.draw_control_frame(id, base, MU_STYLE_BASE, opt,
                                          ctx->_input.get_focus_state(id));
  // draw thumb
  auto w = style->thumb_size;
  auto x = (v - low) * (base.w - w) / (high - low);
  thumb = UIRect(base.x + x, base.y, w, base.h);
  ctx->_command_drawer.draw_control_frame(id, thumb, MU_STYLE_BUTTON, opt,
                                          ctx->_input.get_focus_state(id));
  // draw text
  sprintf(buf, fmt, v);
  ctx->_command_drawer.draw_control_text(buf, base, MU_STYLE_TEXT, opt);

  return res;
}

MU_RES mu_number_ex(mu_Context *ctx, mu_Real *value, mu_Real step,
                    const char *fmt, MU_OPT opt) {
  char buf[MU_MAX_FMT + 1];
  auto res = MU_RES::MU_RES_NONE;
  mu_Id id = ctx->_hash.create(&value, sizeof(value));
  auto style = ctx->_command_drawer.style();
  UIRect base = ctx->_layout.next(style);
  mu_Real last = *value;

  // handle text input mode
  if (number_textbox(ctx, value, base, id)) {
    return res;
  }

  // handle normal mode
  auto mouseover = ctx->mouse_over(base);
  ctx->_input.update_focus_hover(id, opt, mouseover);

  // handle input
  if (ctx->_input.has_focus(id) && ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
    *value += ctx->_input.mouse_delta().x * step;
  }
  // set flag if value changed
  if (*value != last) {
    res = res | MU_RES_CHANGE;
  }

  // draw base
  ctx->_command_drawer.draw_control_frame(id, base, MU_STYLE_BASE, opt,
                                          ctx->_input.get_focus_state(id));
  // draw text
  sprintf(buf, fmt, *value);
  ctx->_command_drawer.draw_control_text(buf, base, MU_STYLE_TEXT, opt);

  return res;
}

static MU_RES header(mu_Context *ctx, const char *label, int istreenode,
                     MU_OPT opt) {
  mu_Id id = ctx->_hash.create(label, strlen(label));
  int idx = ctx->treenode_pool.get_index(id);
  int width = -1;
  ctx->_layout.back().row(1, &width, 0);

  auto active = (idx >= 0);
  auto expanded = (opt & MU_OPT_EXPANDED) ? !active : active;
  auto style = ctx->_command_drawer.style();
  auto r = ctx->_layout.next(style);
  auto mouseover = ctx->mouse_over(r);
  ctx->_input.update_focus_hover(id, MU_OPT::MU_OPT_NONE, mouseover);

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
      ctx->_command_drawer.draw_frame(r, MU_STYLE_BUTTONHOVER);
    }
  } else {
    ctx->_command_drawer.draw_control_frame(id, r, MU_STYLE_BUTTON,
                                            MU_OPT::MU_OPT_NONE,
                                            ctx->_input.get_focus_state(id));
  }
  ctx->_command_drawer.draw_icon(
      ctx, expanded ? MU_ICON_EXPANDED : MU_ICON_COLLAPSED,
      UIRect(r.x, r.y, r.h, r.h), MU_STYLE_TEXT);
  r.x += r.h - style->padding;
  r.w -= r.h - style->padding;
  ctx->_command_drawer.draw_control_text(label, r, MU_STYLE_TEXT,
                                         MU_OPT::MU_OPT_NONE);

  return expanded ? MU_RES_ACTIVE : MU_RES_NONE;
}

MU_RES mu_header_ex(mu_Context *ctx, const char *label, MU_OPT opt) {
  return header(ctx, label, 0, opt);
}

MU_RES mu_begin_treenode_ex(mu_Context *ctx, const char *label, MU_OPT opt) {
  auto res = header(ctx, label, 1, opt);
  if (res & MU_RES_ACTIVE) {
    auto style = ctx->_command_drawer.style();
    ctx->_layout.back().indent += style->indent;
    ctx->_hash.push_last();
  }
  return res;
}

void mu_end_treenode(mu_Context *ctx) {
  auto style = ctx->_command_drawer.style();
  ctx->_layout.back().indent -= style->indent;
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
    auto style = ctx->_command_drawer.style();
    base.w = style->scrollbar_size;

    // handle input
    auto mouseover = ctx->mouse_over(base);
    ctx->_input.update_focus_hover(id, MU_OPT_NONE, mouseover);
    if (ctx->_input.has_focus(id) &&
        ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
      cnt->scroll.y += ctx->_input.mouse_delta().y * cs.y / base.h;
    }
    // clamp scroll to limits
    cnt->scroll.y = mu_clamp(cnt->scroll.y, 0, maxscroll);

    // draw base and thumb
    ctx->_command_drawer.draw_frame(base, MU_STYLE_SCROLLBASE);
    thumb = base;
    thumb.h = mu_max(style->thumb_size, base.h * b->h / cs.y);
    thumb.y += cnt->scroll.y * (base.h - thumb.h) / maxscroll;
    ctx->_command_drawer.draw_frame(thumb, MU_STYLE_SCROLLTHUMB);

    // set this as the scroll_target (will get scrolled on mousewheel)
    // if the mouse is over it
    if (ctx->mouse_over(*b)) {
      ctx->_input.set_scroll_target(cnt);
    }
  } else {
    cnt->scroll.y = 0;
  }
}

static void scrollbars(mu_Context *ctx, mu_Container *cnt, UIRect *body) {
  auto style = ctx->_command_drawer.style();
  int sz = style->scrollbar_size;
  UIVec2 cs = cnt->content_size;
  cs.x += style->padding * 2;
  cs.y += style->padding * 2;
  ctx->_command_drawer.push_clip(*body);
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
  ctx->_command_drawer.pop_clip();
}

static void push_container_body(mu_Context *ctx, mu_Container *cnt, UIRect body,
                                MU_OPT opt) {
  if (~opt & MU_OPT_NOSCROLL) {
    scrollbars(ctx, cnt, &body);
  }
  auto style = ctx->_command_drawer.style();
  ctx->_layout.push(mu_Layout(body.expand(-style->padding), cnt->scroll));
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

  ctx->_container.begin_root_container(cnt, ctx->_command_drawer.size(),
                                       ctx->_input.mouse_pos());
  /* clipping is reset here in case a root-container is made within
  ** another root-containers's begin/end block; this prevents the inner
  ** root-container being clipped to the outer */
  ctx->_command_drawer.push_unclipped_rect();

  auto body = cnt->rect;
  rect = body;

  // draw frame
  if (~opt & MU_OPT_NOFRAME) {
    ctx->_command_drawer.draw_frame(rect, MU_STYLE_WINDOWBG);
  }

  // do title bar
  if (~opt & MU_OPT_NOTITLE) {
    UIRect tr = rect;
    auto style = ctx->_command_drawer.style();
    tr.h = style->title_height;
    ctx->_command_drawer.draw_frame(tr, MU_STYLE_TITLEBG);

    // do title text
    if (~opt & MU_OPT_NOTITLE) {
      mu_Id id = ctx->_hash.create("!title", 6);
      auto mouseover = ctx->mouse_over(tr);
      ctx->_input.update_focus_hover(id, opt, mouseover);
      ctx->_command_drawer.draw_control_text(title, tr, MU_STYLE_TITLETEXT,
                                             opt);
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
      ctx->_command_drawer.draw_icon(ctx, MU_ICON_CLOSE, r, MU_STYLE_TITLETEXT);
      auto mouseover = ctx->mouse_over(r);
      ctx->_input.update_focus_hover(id, opt, mouseover);
      if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT &&
          ctx->_input.has_focus(id)) {
        cnt->open = false;
      }
    }
  }

  push_container_body(ctx, cnt, body, opt);

  // do `resize` handle
  if (~opt & MU_OPT_NORESIZE) {
    auto style = ctx->_command_drawer.style();
    int sz = style->title_height;
    mu_Id id = ctx->_hash.create("!resize", 7);
    UIRect r = UIRect(rect.x + rect.w - sz, rect.y + rect.h - sz, sz, sz);
    auto mouseover = ctx->mouse_over(r);
    ctx->_input.update_focus_hover(id, opt, mouseover);
    if (ctx->_input.has_focus(id) &&
        ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
      cnt->rect.w = mu_max(96, cnt->rect.w + ctx->_input.mouse_delta().x);
      cnt->rect.h = mu_max(64, cnt->rect.h + ctx->_input.mouse_delta().y);
    }
  }

  // resize to content size
  if (opt & MU_OPT_AUTOSIZE) {
    UIRect r = ctx->_layout.back().body;
    cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
    cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
  }

  // close if this is a popup window and elsewhere was clicked
  if (opt & MU_OPT_POPUP && ctx->_input.mouse_pressed()) {
    if (!ctx->_container.is_hover_root(cnt)) {
      cnt->open = false;
    }
  }

  ctx->_command_drawer.push_clip(cnt->body);
  return MU_RES_ACTIVE;
}

void mu_end_window(mu_Context *ctx) {
  ctx->_command_drawer.pop_clip();
  /* push tail 'goto' jump command and set head 'skip' command. the final steps
  ** on initing these are done in mu_end() */
  auto cnt = ctx->_container.current_container();
  cnt->range.tail = ctx->_command_drawer.size();
  // pop base clip rect and container
  ctx->_command_drawer.pop_clip();
  auto layout = ctx->_layout.pop();
  // auto cnt = ctx->_container.current_container();
  cnt->content_size.x = layout->max.x - layout->body.x;
  cnt->content_size.y = layout->max.y - layout->body.y;
  ctx->_container.pop();
  ctx->_hash.pop();
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
  auto style = ctx->_command_drawer.style();
  cnt->rect = ctx->_layout.next(style);
  if (~opt & MU_OPT_NOFRAME) {
    ctx->_command_drawer.draw_frame(cnt->rect, MU_STYLE_PANELBG);
  }
  ctx->_container.push(cnt);
  push_container_body(ctx, cnt, cnt->rect, opt);
  ctx->_command_drawer.push_clip(cnt->body);
}

void mu_end_panel(mu_Context *ctx) {
  ctx->_command_drawer.pop_clip();
  auto layout = ctx->_layout.pop();
  auto cnt = ctx->_container.current_container();
  cnt->content_size.x = layout->max.x - layout->body.x;
  cnt->content_size.y = layout->max.y - layout->body.y;
  ctx->_container.pop();
  ctx->_hash.pop();
}
