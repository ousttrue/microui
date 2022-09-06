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

mu_Context *mu_new(text_width_callback text_width_callback,
                   text_height_callback text_height_callback) {
  auto ctx = new mu_Context;
  ctx->_command_drawer.style()->text_width_callback = text_width_callback;
  ctx->_command_drawer.style()->text_height_callback = text_height_callback;
  return ctx;
}
void mu_delete(mu_Context *ctx) { delete ctx; }

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
