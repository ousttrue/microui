const std = @import("std");
const c = @import("c");
const Rect = @import("./Rect.zig");
const Vec2 = @import("./Vec2.zig");
const Color32 = @import("./Color32.zig");
const Hash = @import("./Hash.zig");
const Container = @import("./Container.zig");
const ContainerManager = @import("./ContainerManager.zig");
const CommandDrawer = @import("./CommandDrawer.zig");
const Input = @import("./Input.zig");
const RES = enum(u32) {
    ACTIVE = (1 << 0),
    SUBMIT = (1 << 1),
    CHANGE = (1 << 2),
};

const ROOTLIST_SIZE = 32;
const COMMANDLIST_SIZE = 256 * 1024;

const Self = @This();

frame: u32 = 0,
hash: Hash = .{},
container: ContainerManager = .{},
command_drawer: CommandDrawer = .{},
input: Input = .{},

command_groups: [ROOTLIST_SIZE]c.struct_UICommandRange = undefined,
command_buffer: [COMMANDLIST_SIZE]u8 = undefined,

pub fn mouse_over(self: Self, rect: Rect) bool {
    if (!rect.overlaps_vec2(self.input.mouse_pos)) {
        return false;
    }
    if (!self.command_drawer.clip_stack.overlaps(self.input.mouse_pos)) {
        return false;
    }
    if (!self.container.in_hover_root()) {
        return false;
    }
    return true;
}

pub fn begin(self: *Self) void {
    self.command_drawer.begin();
    self.container.begin();
    self.input.begin();
    self.frame += 1;
}

pub fn end(self: *Self, command: *c.UIRenderFrame) !void {
    self.hash.end();
    //   self.layout.end();
    const mouse_pressed = self.input.end();
    self.container.end(mouse_pressed, command);
    self.command_drawer.end(command);
}

pub fn scrollbar(self: *Self, cnt: *Container, b: *Rect, cs: Vec2, key: []const u8) void {
    // only add scrollbar if content size is larger than body
    const maxscroll = cs.y - b.h;
    if (maxscroll > 0 and b.h > 0) {
        // UIRect base, thumb;
        const id = self.hash.create(key);

        // get sizing / positioning
        var base = Rect{};
        base = b.*;
        base.x = b.x + b.w;
        const style = self.command_drawer.style;
        base.w = style.scrollbar_size;

        // handle input
        const mouseover = self.mouse_over(base);
        self.input.update_focus_hover(id, .NONE, mouseover);
        if (self.input.has_focus(id) and
            self.input.mouse_down == .LEFT)
        {
            cnt.scroll.y += self.input.mouse_delta.y * @divTrunc(cs.y, base.h);
        }
        // clamp scroll to limits
        cnt.scroll.y = std.math.clamp(cnt.scroll.y, 0, maxscroll);

        // draw base and thumb
        self.command_drawer.draw_frame(base, .SCROLLBASE);
        var thumb = base;
        thumb.h = std.math.max(style.thumb_size, base.h * @divTrunc(b.h, cs.y));
        thumb.y += @divTrunc(cnt.scroll.y * (base.h - thumb.h), maxscroll);
        self.command_drawer.draw_frame(thumb, .SCROLLTHUMB);

        // set this as the scroll_target (will get scrolled on mousewheel)
        // if the mouse is over it
        if (self.mouse_over(b.*)) {
            self.input.set_scroll_target(cnt);
        }
    } else {
        cnt.scroll.y = 0;
    }
}

pub fn scrollbars(self: *Self, cnt: *Container, body: *Rect) void {
    self.command_drawer.clip_stack.push(body.*);
    defer self.command_drawer.clip_stack.pop();

    // resize body to make room for scrollbars
    const style = self.command_drawer.style;
    const sz = style.scrollbar_size;
    var cs = cnt.content_size;
    cs.x += style.padding * 2;
    cs.y += style.padding * 2;
    if (cs.y > cnt.body.h) {
        body.w -= sz;
    }
    if (cs.x > cnt.body.w) {
        body.h -= sz;
    }

    // to create a horizontal or vertical scrollbar almost-identical code is
    // used; only the references to `x|y` `w|h` need to be switched */
    self.scrollbar(cnt, body, cs, "!scrollbary"); // x, y, w, h);
    // scrollbar(ctx, cnt, body, cs, "!scrollbarx"); // y, x, h, w);
}

pub fn begin_window(self: *Self, title: []const u8, rect: Rect, opt: Input.OPT) ?RES {
    const id = self.hash.create(title);
    const cnt = self.container.get_container(id, opt, self.frame) orelse {
        return null;
    };
    if (!cnt.open) {
        return null;
    }
    self.hash.stack.push(id);

    if (cnt.rect.w == 0) {
        // first time
        cnt.rect = rect;
    }

    self.container.begin_root_container(cnt, self.command_drawer.pos, self.input.mouse_pos);
    // clipping is reset here in case a root-container is made within
    // another root-containers's begin/end block; this prevents the inner
    // root-container being clipped to the outer */
    self.command_drawer.clip_stack.push_unclipped_rect();

    // draw frame
    if (!opt.has(.NOFRAME)) {
        self.command_drawer.draw_frame(cnt.rect, .WINDOWBG);
    }

    // do title bar
    var body = cnt.rect;
    if (!opt.has(.NOTITLE)) {
        var tr = cnt.rect;
        const style = self.command_drawer.style;
        tr.h = style.title_height;
        self.command_drawer.draw_frame(tr, .TITLEBG);

        // do title text
        {
            const title_id = self.hash.create("!title");
            const mouseover = self.mouse_over(tr);
            self.input.update_focus_hover(title_id, opt, mouseover);
            self.command_drawer.draw_control_text(title, tr, .TITLETEXT, opt);
            if (self.input.has_focus(title_id) and
                self.input.mouse_down == .LEFT)
            {
                // drag
                cnt.rect.x += self.input.mouse_delta.x;
                cnt.rect.y += self.input.mouse_delta.y;
            }
            body.y += tr.h;
            body.h -= tr.h;
        }

        // do `close` button
        if (!opt.has(.NOCLOSE)) {
            const close_id = self.hash.create("!close");
            const r = Rect{ .x = tr.x + tr.w - tr.h, .y = tr.y, .w = tr.h, .h = tr.h };
            tr.w -= r.w;
            self.command_drawer.draw_icon(c.MU_ICON_CLOSE, r, .TITLETEXT);
            const mouseover = self.mouse_over(r);
            self.input.update_focus_hover(close_id, opt, mouseover);
            if (self.input.mouse_pressed == .LEFT and
                self.input.has_focus(close_id))
            {
                // close
                cnt.open = false;
            }
        }
    }

    if (!opt.has(.NOSCROLL)) {
        self.scrollbars(cnt, &body);
    }
    const style = self.command_drawer.style;
    // self.layout.push(mu_Layout(body.expand(-style.padding), cnt.scroll));
    cnt.body = body;

    // do `resize` handle
    if (!opt.has(.NORESIZE)) {
        const sz = style.title_height;
        const resize_id = self.hash.create("!resize");
        const r = Rect{
            .x = cnt.rect.x + cnt.rect.w - sz,
            .y = cnt.rect.y + cnt.rect.h - sz,
            .w = sz,
            .h = sz,
        };
        const mouseover = self.mouse_over(r);
        self.input.update_focus_hover(resize_id, opt, mouseover);
        if (self.input.has_focus(resize_id) and
            self.input.mouse_down == .LEFT)
        {
            cnt.rect.w = std.math.max(96, cnt.rect.w + self.input.mouse_delta.x);
            cnt.rect.h = std.math.max(64, cnt.rect.h + self.input.mouse_delta.y);
        }
    }

    //   // resize to content size
    //   if (opt & MU_OPT_AUTOSIZE) {
    //     UIRect r = self.layout.back().body;
    //     cnt.rect.w = cnt.content_size.x + (cnt.rect.w - r.w);
    //     cnt.rect.h = cnt.content_size.y + (cnt.rect.h - r.h);
    //   }

    // close if this is a popup window and elsewhere was clicked
    if (opt.has(.POPUP) and self.input.mouse_pressed != .NONE) {
        if (!self.container.is_hover_root(cnt)) {
            cnt.open = false;
        }
    }

    self.command_drawer.clip_stack.push(cnt.body);
    return .ACTIVE;
}

pub fn end_window(self: *Self) void {
    self.command_drawer.clip_stack.pop();
    // push tail 'goto' jump command and set head 'skip' command. the final steps
    // on initing these are done in mu_end()
    const cnt = self.container.current_container();
    cnt.tail = self.command_drawer.pos;
    // pop base clip rect and container
    self.command_drawer.clip_stack.pop();

    // apply layout size
    // const layout = self.layout.pop();
    // // auto cnt = self.container.current_container();
    // cnt.content_size.x = layout.max.x - layout.body.x;
    // cnt.content_size.y = layout.max.y - layout.body.y;

    self.container.pop();
    self.hash.pop();
}
