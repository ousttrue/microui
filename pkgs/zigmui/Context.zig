const std = @import("std");
const c = @import("c");
const Rect = @import("./Rect.zig");
const Color32 = @import("./Color32.zig");
const Hash = @import("./Hash.zig");
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

    var body = cnt.rect;
    // rect = body;

    // draw frame
    if (!opt.contains(.NOFRAME)) {
        self.command_drawer.draw_frame(cnt.rect, .WINDOWBG);
    }

    // do title bar
    if (!opt.contains(.NOTITLE)) {
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
        if (!opt.contains(.NOCLOSE)) {
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

    //   push_container_body(ctx, cnt, body, opt);

    //   // do `resize` handle
    //   if (~opt & MU_OPT_NORESIZE) {
    //     auto style = self.command_drawer.style();
    //     int sz = style.title_height;
    //     mu_Id id = self.hash.create("!resize", 7);
    //     UIRect r = UIRect(rect.x + rect.w - sz, rect.y + rect.h - sz, sz, sz);
    //     auto mouseover = ctx.mouse_over(r);
    //     self.input.update_focus_hover(id, r, opt, mouseover);
    //     if (self.input.has_focus(id) &&
    //         self.input.mouse_down() == MU_MOUSE_LEFT) {
    //       cnt.rect.w = mu_max(96, cnt.rect.w + self.input.mouse_delta().x);
    //       cnt.rect.h = mu_max(64, cnt.rect.h + self.input.mouse_delta().y);
    //     }
    //   }

    //   // resize to content size
    //   if (opt & MU_OPT_AUTOSIZE) {
    //     UIRect r = self.layout.back().body;
    //     cnt.rect.w = cnt.content_size.x + (cnt.rect.w - r.w);
    //     cnt.rect.h = cnt.content_size.y + (cnt.rect.h - r.h);
    //   }

    //   // close if this is a popup window and elsewhere was clicked
    //   if (opt & MU_OPT_POPUP && self.input.mouse_pressed()) {
    //     if (!self.container.is_hover_root(cnt)) {
    //       cnt.open = false;
    //     }
    //   }

    //   self.command_drawer.push_clip(cnt.body);
    return .ACTIVE;
}

pub fn end_window(self: *Self) void {
    self.command_drawer.clip_stack.pop();
    // push tail 'goto' jump command and set head 'skip' command. the final steps
    // on initing these are done in mu_end()
    const cnt = self.container.current_container();
    cnt.tail = self.command_drawer.pos;
    // pop base clip rect and container
    // self.command_drawer.pop_clip();
    // const layout = self.layout.pop();
    // // auto cnt = self.container.current_container();
    // cnt.content_size.x = layout.max.x - layout.body.x;
    // cnt.content_size.y = layout.max.y - layout.body.y;
    self.container.pop();
    self.hash.pop();
}
