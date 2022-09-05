const std = @import("std");
const c = @import("c");
const Context = @import("./Context.zig");
const Rect = @import("./Rect.zig");
const Input = @import("./Input.zig");
const Layout = @import("./Layout.zig");
const OPT = Input.OPT;

const RES = enum(u32) {
    ACTIVE = (1 << 0),
    SUBMIT = (1 << 1),
    CHANGE = (1 << 2),
};

pub fn label(ctx: *Context, text: []const u8) void {
    const style = ctx.command_drawer.style;
    ctx.command_drawer.draw_control_text(text, ctx.layout.next(style), .TEXT, .NONE);
}

pub fn begin_window(ctx: *Context, title: []const u8, rect: Rect, opt: OPT) ?RES {
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
            ctx.command_drawer.draw_control_text(title, tr, .TITLETEXT, opt);
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
    ctx.layout.layout_stack.push(Layout.create(body.expand(-style.padding), cnt.scroll));
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
        const r = ctx.layout.layout_stack.back_const().body;
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
    const layout = ctx.layout.layout_stack.back_const();
    ctx.layout.layout_stack.pop();
    cnt.content_size.x = layout.max.x - layout.body.x;
    cnt.content_size.y = layout.max.y - layout.body.y;

    ctx.container.pop();
    ctx.hash.stack.pop();
}
