const std = @import("std");
const c = @import("c");
const Context = @import("./Context.zig");
const Vec2 = @import("./Vec2.zig");
const Rect = @import("./Rect.zig");
const Input = @import("./Input.zig");
const Layout = @import("./Layout.zig");
const Container = @import("./Container.zig");
const Hash = @import("./Hash.zig");
const TextEditor = @import("./TextEditor.zig");
const CommandDrawer = @import("./CommandDrawer.zig");
const Style = @import("./Style.zig");
const OPT = Input.OPT;

pub fn label(ctx: *Context, text: []const u8) void {
    const style = &ctx.command_drawer.style;
    ctx.command_drawer.draw_control_text(text, ctx.layout.back().next(style), .TEXT, .NONE, false);
}

pub fn textbox_raw(ctx: *Context, buf: []u8, id: Hash.Id, rect: Rect, opt: OPT) Input.RES {
    // base rect
    const mouseover = ctx.mouse_over(rect);
    ctx.input.update_focus_hover(id, opt.add(.HOLDFOCUS), mouseover);
    ctx.command_drawer.draw_control_frame(rect, .BASE, opt, ctx.input.get_focus_state(id));

    // base rect
    var res = Input.RES.NONE;
    if (ctx.input.has_focus(id)) {
        // text editor
        const handled = ctx.input.handle_text(id, buf);
        res = res.add(handled.res);
        ctx.command_drawer.draw_control_text(buf[0..handled.size], rect, .TEXT, opt, true);
    } else {
        const len = c.strlen(&buf[0]);
        ctx.command_drawer.draw_control_text(buf[0..len], rect, .TEXT, opt, false);
    }
    return res;
}

pub fn textbox(ctx: *Context, whole_buf: []u8, option: struct { opt: Input.OPT = .NONE }) Input.RES {
    const id = ctx.hash.from_value(&whole_buf[0]);
    const style = &ctx.command_drawer.style;
    const r = ctx.layout.back().next(style);

    return textbox_raw(ctx, whole_buf, id, r, option.opt);
}

fn get_word(src: []const u8) []const u8 {
    for (src) |ch, i| {
        if (ch == 0 or ch == ' ' or ch == '\n') {
            return src[0..i];
        }
    }
    return src;
}

const Line = struct {
    text: []const u8,
    pos: Vec2,
};

const LineIterator = struct {
    const Self = @This();

    src: []const u8,
    style: *const Style,
    pos: usize = 0,

    pub fn init(src: []const u8, style: *const Style) Self {
        return Self{
            .src = src,
            .style = style,
        };
    }

    pub fn next(self: *Self, rect: Rect) ?Line {
        if (self.pos >= self.src.len) {
            return null;
        }

        const head = self.pos;

        var w: i32 = 0;
        while (self.pos < self.src.len) {
            const word = get_word(self.src[self.pos..]);
            const word_width = self.style.text_width(word);
            if (w + word_width > rect.w) {
                // wrap
                break;
            }
            // advance
            self.pos += word.len;
            w += word_width;

            // space
            if (self.pos < self.src.len) {
                const ch = self.src[self.pos];
                const space = self.src[self.pos .. self.pos + 1];
                self.pos += 1;
                if (ch == '\n') {
                    break;
                }
                w += self.style.text_width(space);
            }
        }

        return Line{
            .text = self.src[head..self.pos],
            .pos = .{ .x = rect.x, .y = rect.y },
        };
    }
};

pub fn textarea(ctx: *Context, src: []const u8) void {
    const style = &ctx.command_drawer.style;
    var layout = ctx.layout.begin_column(style);
    layout.row(&.{-1}, style.text_height());

    var it = LineIterator.init(src, style);
    while (it.next(layout.next(style))) |line| {
        ctx.command_drawer.draw_text(line.text, line.pos, .TEXT);
    }

    ctx.layout.end_column();
}

pub fn button(
    ctx: *Context,
    value: union(enum) { text: []const u8, icon: CommandDrawer.ICON },
    option: struct { opt: Input.OPT = .NONE },
) bool {
    var res = false;
    const id = switch (value) {
        .text => |text| ctx.hash.from_str(text),
        .icon => |icon| ctx.hash.from_value(icon),
    };
    const style = &ctx.command_drawer.style;
    const rect = ctx.layout.back().next(style);
    const mouseover = ctx.mouse_over(rect);
    ctx.input.update_focus_hover(id, option.opt, mouseover);
    // handle click
    if (ctx.input.mouse_pressed == .LEFT and ctx.input.has_focus(id)) {
        res = true;
    }
    // draw
    ctx.command_drawer.draw_control_frame(rect, .BUTTON, option.opt, ctx.input.get_focus_state(id));
    switch (value) {
        .text => |text| ctx.command_drawer.draw_control_text(text, rect, .TEXT, option.opt, false),
        .icon => |icon| ctx.command_drawer.draw_icon(icon, rect, .TEXT),
    }
    return res;
}

pub fn checkbox(ctx: *Context, text: []const u8, state: *bool) bool {
    const id = ctx.hash.from_value(state);
    const style = &ctx.command_drawer.style;
    const r = ctx.layout.back().next(style);
    const box = Rect{ .x = r.x, .y = r.y, .w = r.h, .h = r.h };
    const mouseover = ctx.mouse_over(r);
    ctx.input.update_focus_hover(id, .NONE, mouseover);

    // handle click
    var res = false;
    if (ctx.input.mouse_pressed == .LEFT and ctx.input.has_focus(id)) {
        res = true;
        state.* = !state.*;
    }

    // draw
    ctx.command_drawer.draw_control_frame(box, .BASE, .NONE, ctx.input.get_focus_state(id));
    if (state.*) {
        ctx.command_drawer.draw_icon(.CHECK, box, .TEXT);
    }
    const text_rect = Rect{ .x = r.x + box.w, .y = r.y, .w = r.w - box.w, .h = r.h };
    ctx.command_drawer.draw_control_text(text, text_rect, .TEXT, .NONE, false);

    return res;
}

pub fn scrollbar(ctx: *Context, cnt: *Container, b: *Rect, cs: Vec2, key: []const u8) void {
    const maxscroll = cs.y - b.h;
    if (maxscroll > 0 and b.h > 0) {
        // only add scrollbar if content size is larger than body
        const id = ctx.hash.from_str(key);

        // get sizing / positioning
        var base = b.*;
        base.x = b.x + b.w;
        const style = &ctx.command_drawer.style;
        base.w = style.scrollbar_size;

        // handle input
        const mouseover = ctx.mouse_over(base);
        ctx.input.update_focus_hover(id, .NONE, mouseover);
        if (ctx.input.has_focus(id) and ctx.input.mouse_down == .LEFT) {
            cnt.scroll.y += @floatToInt(
                i32,
                @intToFloat(f32, ctx.input.mouse_delta.y) * @intToFloat(f32, cs.y) / @intToFloat(f32, base.h),
            );
        }

        // clamp scroll to limits
        cnt.scroll.y = std.math.clamp(cnt.scroll.y, 0, maxscroll);

        // draw base and thumb
        ctx.command_drawer.draw_frame(base, .SCROLLBASE);
        var thumb = base;
        thumb.h = std.math.max(style.thumb_size, @floatToInt(i32, @intToFloat(f32, base.h) * @intToFloat(f32, b.h) / @intToFloat(f32, cs.y)));
        thumb.y += @floatToInt(i32, @intToFloat(f32, cnt.scroll.y) * @intToFloat(f32, base.h - thumb.h) / @intToFloat(f32, maxscroll));
        ctx.command_drawer.draw_frame(thumb, .SCROLLTHUMB);

        // set this as the scroll_target (will get scrolled on mousewheel)
        // if the mouse is over it
        if (ctx.mouse_over(b.*)) {
            ctx.input.set_scroll_target(cnt);
        }
    } else {
        cnt.scroll.y = 0;
    }
}

pub fn scrollbars(ctx: *Context, cnt: *Container, body: *Rect) void {
    const style = &ctx.command_drawer.style;
    const sz = style.scrollbar_size;
    var cs = cnt.content_size;
    cs.x += style.padding * 2;
    cs.y += style.padding * 2;
    ctx.command_drawer.clip_stack.push(body.*);
    // resize body to make room for scrollbars
    if (cs.y > cnt.body.h) {
        body.w -= sz;
    }
    if (cs.x > cnt.body.w) {
        body.h -= sz;
    }
    // to create a horizontal or vertical scrollbar almost-identical code is
    // used; only the references to `x|y` `w|h` need to be switched
    scrollbar(ctx, cnt, body, cs, "!scrollbary"); // x, y, w, h);
    // scrollbar(ctx, cnt, body, cs, "!scrollbarx"); // y, x, h, w);
    ctx.command_drawer.clip_stack.pop();
}

pub fn begin_window(ctx: *Context, text: []const u8, rect: Rect, opt: OPT) bool {
    const id = ctx.hash.from_str(text);
    const cnt = ctx.container.get_container(id, opt, ctx.frame) orelse {
        return false;
    };
    if (!cnt.open) {
        return false;
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
            ctx.command_drawer.draw_control_text(text, tr, .TITLETEXT, opt, false);
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
            ctx.command_drawer.draw_icon(.CLOSE, r, .TITLETEXT);
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
        scrollbars(ctx, cnt, &body);
    }
    const style = ctx.command_drawer.style;
    ctx.layout.stack.push(Layout.fromRect(body.expand(-style.padding).move(cnt.scroll)));
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
    return true;
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

pub fn open_popup(ctx: *Context, name: []const u8) void {
    const id = ctx.hash.from_str(name);
    ctx.container.open_popup(id, ctx.input.mouse_pos, ctx.frame);
}

pub fn begin_popup(ctx: *Context, name: []const u8) bool {
    const opt = Input.OPT.POPUP.add(.AUTOSIZE).add(.NORESIZE).add(.NOSCROLL).add(.NOTITLE).add(.CLOSED);
    return begin_window(ctx, name, .{}, opt);
}

pub fn end_popup(ctx: *Context) void {
    end_window(ctx);
}

pub fn begin_panel(ctx: *Context, name: []const u8, option: struct { opt: Input.OPT = .NONE }) void {
    const last_id = ctx.hash.from_str(name);
    ctx.hash.stack.push(last_id);
    if (ctx.container.get_container(last_id, option.opt, ctx.frame)) |cnt| {
        const style = &ctx.command_drawer.style;
        cnt.rect = ctx.layout.back().next(style);
        if (!option.opt.has(.NOFRAME)) {
            ctx.command_drawer.draw_frame(cnt.rect, .PANELBG);
        }
        ctx.container.container_stack.push(cnt);
        if (!option.opt.has(.NOSCROLL)) {
            scrollbars(ctx, cnt, &cnt.rect);
        }
        ctx.layout.push(Layout.fromRect(cnt.rect.expand(-style.padding).move(cnt.scroll)));
        cnt.body = cnt.rect;
        ctx.command_drawer.clip_stack.push(cnt.body);
    }
}

pub fn end_panel(ctx: *Context) void {
    ctx.command_drawer.clip_stack.pop();
    const layout = ctx.layout.pop();
    var cnt = ctx.container.current_container();
    cnt.content_size = layout.remain();
    ctx.container.pop();
    ctx.hash.stack.pop();
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

pub fn slider(
    ctx: *Context,
    value: *f32,
    low: f32,
    high: f32,
    step: f32,
    comptime fmt: ?[]const u8,
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
    if (fmt) |f| {
        var buf: [127 + 1]u8 = undefined;
        const slice = if (std.fmt.bufPrint(&buf, f, .{v})) |slice|
            slice
        else |_|
            buf[0..0];
        ctx.command_drawer.draw_control_text(slice, base, .TEXT, opt, false);
    }

    return res;
}

pub fn header(ctx: *Context, title: []const u8, option: struct { istreenode: bool = false, opt: Input.OPT = .NONE }) bool {
    const id = ctx.hash.from_str(title);
    const idx = ctx.tree.get(id);
    ctx.layout.back().row(&.{-1}, 0);
    var active = (idx != null);
    const expanded = if (option.opt.has(.EXPANDED)) !active else active;
    const style = &ctx.command_drawer.style;
    var rect = ctx.layout.back().next(style);
    const mouseover = ctx.mouse_over(rect);
    ctx.input.update_focus_hover(id, .NONE, mouseover);

    // handle click
    active = active != (ctx.input.mouse_pressed == .LEFT and ctx.input.has_focus(id));

    // update pool ref
    ctx.tree.update(id, idx, active, ctx.frame);

    // draw
    if (option.istreenode) {
        if (ctx.input.has_hover(id)) {
            ctx.command_drawer.draw_frame(rect, .BUTTONHOVER);
        }
    } else {
        ctx.command_drawer.draw_control_frame(rect, .BUTTON, .NONE, ctx.input.get_focus_state(id));
    }
    ctx.command_drawer.draw_icon(
        if (expanded) .COLLAPSED else .EXPANDED,
        Rect{ .x = rect.x, .y = rect.y, .w = rect.h, .h = rect.h },
        .TEXT,
    );
    rect.x += rect.h - style.padding;
    rect.w -= rect.h - style.padding;
    ctx.command_drawer.draw_control_text(title, rect, .TEXT, .NONE, false);

    return expanded;
}

pub fn begin_treenode(ctx: *Context, text: []const u8, option: struct { opt: Input.OPT = .NONE }) bool {
    var res = header(ctx, text, .{ .istreenode = true, .opt = option.opt });
    if (res) {
        const style = &ctx.command_drawer.style;
        ctx.layout.back().indent += style.indent;
        ctx.hash.push_last();
    }
    return res;
}

pub fn end_treenode(ctx: *Context) void {
    const style = &ctx.command_drawer.style;
    ctx.layout.back().indent -= style.indent;
    ctx.hash.stack.pop();
}
