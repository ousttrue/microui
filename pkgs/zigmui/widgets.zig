const std = @import("std");
const c = @import("c");
const Context = @import("./Context.zig");
const Rect = @import("./Rect.zig");
const Input = @import("./Input.zig");
const Layout = @import("./Layout.zig");
const Hash = @import("./Hash.zig");
const OPT = Input.OPT;

pub fn label(ctx: *Context, text: []const u8) void {
    const style = &ctx.command_drawer.style;
    ctx.command_drawer.draw_control_text(text, ctx.layout.back().next(style), .TEXT, .NONE, false);
}

pub fn begin_window(ctx: *Context, title: []const u8, rect: Rect, opt: OPT) ?Input.RES {
    const id = ctx.hash.from_str(title);
    const cnt = ctx.container.get_container(id, opt, ctx.frame) orelse {
        return null;
    };
    if (!cnt.open) {
        return null;
    }
    ctx.hash.stack.push(id);

    if (cnt.rect.w == 0) {
        // first time
        cnt.rect = rect;
    }

    ctx.container.begin_root_container(cnt, ctx.command_drawer.pos, ctx.input.mouse_pos);
    // clipping is reset here in case a root-container is made within
    // another root-containers's begin/end block; this prevents the inner
    // root-container being clipped to the outer */
    ctx.command_drawer.clip_stack.push_unclipped_rect();

    // draw frame
    if (!opt.has(.NOFRAME)) {
        ctx.command_drawer.draw_frame(cnt.rect, .WINDOWBG);
    }

    // do title bar
    var body = cnt.rect;
    if (!opt.has(.NOTITLE)) {
        var tr = cnt.rect;
        const style = ctx.command_drawer.style;
        tr.h = style.title_height;
        ctx.command_drawer.draw_frame(tr, .TITLEBG);

        // do title text
        {
            const title_id = ctx.hash.from_str("!title");
            const mouseover = ctx.mouse_over(tr);
            ctx.input.update_focus_hover(title_id, opt, mouseover);
            ctx.command_drawer.draw_control_text(title, tr, .TITLETEXT, opt, false);
            if (ctx.input.has_focus(title_id) and
                ctx.input.mouse_down == .LEFT)
            {
                // drag
                cnt.rect.x += ctx.input.mouse_delta.x;
                cnt.rect.y += ctx.input.mouse_delta.y;
            }
            body.y += tr.h;
            body.h -= tr.h;
        }

        // do `close` button
        if (!opt.has(.NOCLOSE)) {
            const close_id = ctx.hash.from_str("!close");
            const r = Rect{ .x = tr.x + tr.w - tr.h, .y = tr.y, .w = tr.h, .h = tr.h };
            tr.w -= r.w;
            ctx.command_drawer.draw_icon(c.MU_ICON_CLOSE, r, .TITLETEXT);
            const mouseover = ctx.mouse_over(r);
            ctx.input.update_focus_hover(close_id, opt, mouseover);
            if (ctx.input.mouse_pressed == .LEFT and
                ctx.input.has_focus(close_id))
            {
                // close
                cnt.open = false;
            }
        }
    }

    if (!opt.has(.NOSCROLL)) {
        ctx.scrollbars(cnt, &body);
    }
    const style = ctx.command_drawer.style;
    ctx.layout.stack.push(Layout.fromRect(body.expand(-style.padding).sub(cnt.scroll)));
    cnt.body = body;

    // do `resize` handle
    if (!opt.has(.NORESIZE)) {
        const sz = style.title_height;
        const resize_id = ctx.hash.from_str("!resize");
        const r = Rect{
            .x = cnt.rect.x + cnt.rect.w - sz,
            .y = cnt.rect.y + cnt.rect.h - sz,
            .w = sz,
            .h = sz,
        };
        const mouseover = ctx.mouse_over(r);
        ctx.input.update_focus_hover(resize_id, opt, mouseover);
        if (ctx.input.has_focus(resize_id) and
            ctx.input.mouse_down == .LEFT)
        {
            cnt.rect.w = std.math.max(96, cnt.rect.w + ctx.input.mouse_delta.x);
            cnt.rect.h = std.math.max(64, cnt.rect.h + ctx.input.mouse_delta.y);
        }
    }

    // resize to content size
    if (opt.has(.AUTOSIZE)) {
        const r = ctx.layout.stack.back_const().body;
        cnt.rect.w = cnt.content_size.x + (cnt.rect.w - r.w);
        cnt.rect.h = cnt.content_size.y + (cnt.rect.h - r.h);
    }

    // close if this is a popup window and elsewhere was clicked
    if (opt.has(.POPUP) and ctx.input.mouse_pressed != .NONE) {
        if (!ctx.container.is_hover_root(cnt)) {
            cnt.open = false;
        }
    }

    ctx.command_drawer.clip_stack.push(cnt.body);
    return .ACTIVE;
}

pub fn end_window(ctx: *Context) void {
    // body
    ctx.command_drawer.clip_stack.pop();

    // push tail 'goto' jump command and set head 'skip' command. the final steps
    // on initing these are done in mu_end()
    var cnt = ctx.container.current_container();
    cnt.tail = ctx.command_drawer.pos;

    // pop base clip rect and container
    ctx.command_drawer.clip_stack.pop();

    // apply layout size
    const layout = ctx.layout.stack.back_const();
    ctx.layout.stack.pop();
    cnt.content_size.x = layout.max.x - layout.body.x;
    cnt.content_size.y = layout.max.y - layout.body.y;

    ctx.container.pop();
    ctx.hash.stack.pop();
}

pub fn textbox_raw(ctx: *Context, buf: []u8, id: Hash.Id, rect: Rect, opt: OPT) Input.RES {
    var res: Input.RES = .NONE;

    // base rect
    const mouseover = ctx.mouse_over(rect);
    ctx.input.update_focus_hover(id, opt.add(.HOLDFOCUS), mouseover);
    ctx.command_drawer.draw_control_frame(rect, .BASE, opt, ctx.input.get_focus_state(id));

    if (ctx.input.has_focus(id)) {
        // text editor
        res = res.add(ctx.input.handle_text(id, buf));
        ctx.command_drawer.draw_control_text(buf, rect, .TEXT, opt, true);
    } else {
        ctx.command_drawer.draw_control_text(buf, rect, .TEXT, opt, false);
    }

    return res;
}

pub fn number_textbox(ctx: *Context, value: *f32, r: Rect, id: Hash.Id) bool {
    if (ctx.input.mouse_pressed == .LEFT and
        ctx.input.key_down.has(.SHIFT) and ctx.input.has_hover(id))
    {
        ctx.editor.set_value(id, value.*);
    }

    if (ctx.editor.buffer(id)) |*buffer| {
        const res = textbox_raw(ctx, buffer.*, id, r, .NONE);
        if (res.has(.SUBMIT) or !ctx.input.has_focus(id)) {
            value.* = ctx.editor.commit();
        } else {
            return true;
        }
    }

    return false;
}

pub fn slider_ex(
    ctx: *Context,
    value: *f32,
    low: f32,
    high: f32,
    step: f32,
    fmt: [:0]const u8,
    opt: OPT,
) Input.RES {
    const last = value.*;
    var v: f32 = last;
    const id = ctx.hash.from_value(value);
    const style = &ctx.command_drawer.style;
    const base = ctx.layout.back().next(style);

    // handle text input mode
    var res: Input.RES = .NONE;
    if (number_textbox(ctx, &v, base, id)) {
        return res;
    }

    // handle normal mode
    const mouseover = ctx.mouse_over(base);
    ctx.input.update_focus_hover(id, opt, mouseover);

    // handle input
    if (ctx.input.has_focus(id) and
        @intToEnum(Input.MOUSE_BUTTON, @enumToInt(ctx.input.mouse_down) | @enumToInt(ctx.input.mouse_pressed)) == .LEFT)
    {
        v = low + @intToFloat(f32, ctx.input.mouse_pos.x - base.x) * (high - low) / @intToFloat(f32, base.w);
        if (step != 0) {
            v = (((v + step / 2) / step)) * step;
        }
    }
    // clamp and store value, update res
    v = std.math.clamp(v, low, high);
    value.* = v;
    if (last != v) {
        res = res.add(.CHANGE);
    }

    // draw base
    ctx.command_drawer.draw_control_frame(base, .BASE, opt, ctx.input.get_focus_state(id));
    // draw thumb
    const w = style.thumb_size;
    const x = @floatToInt(i32, (v - low) * @intToFloat(f32, base.w - w) / (high - low));
    const thumb = Rect{ .x = base.x + x, .y = base.y, .w = w, .h = base.h };
    ctx.command_drawer.draw_control_frame(thumb, .BUTTON, opt, ctx.input.get_focus_state(id));
    // draw text
    var buf: [127 + 1]u8 = undefined;
    const buf_len = c.sprintf(&buf[0], @ptrCast([*:0]const u8, &fmt[0]), v);
    ctx.command_drawer.draw_control_text(buf[0..@intCast(usize, buf_len)], base, .TEXT, opt, false);

    return res;
}
