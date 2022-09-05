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
const LayoutManager = @import("./LayoutManager.zig");
const Layout = @import("./Layout.zig");

const ROOTLIST_SIZE = 32;
const COMMANDLIST_SIZE = 256 * 1024;

const Self = @This();

frame: u32 = 0,
hash: Hash = .{},
container: ContainerManager = .{},
command_drawer: CommandDrawer = .{},
input: Input = .{},
layout: LayoutManager = .{},

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
    self.layout.end();
    const mouse_pressed = self.input.end();
    self.container.end(mouse_pressed, command);
    self.command_drawer.end(command);
}

pub fn scrollbar(self: *Self, cnt: *Container, b: *Rect, cs: Vec2, key: []const u8) void {
    // only add scrollbar if content size is larger than body
    const maxscroll = cs.y - b.h;
    if (maxscroll > 0 and b.h > 0) {
        // UIRect base, thumb;
        const id = self.hash.from_str(key);

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
