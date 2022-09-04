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

pub const text_width_callback = fn (_: ?*anyopaque, text: [*c]const u8, _len: c_int) callconv(.C) c_int;
pub const text_height_callback = fn (_: ?*anyopaque) callconv(.C) c_int;
const ROOTLIST_SIZE = 32;
const COMMANDLIST_SIZE = 256 * 1024;

const Self = @This();

frame: u32 = 0,
hash: Hash = .{},
container: ContainerManager = .{},
command_drawer: CommandDrawer = .{},
input: Input = .{},

text_width: ?*const text_width_callback,
text_height: ?*const text_height_callback,
command_groups: [ROOTLIST_SIZE]c.struct_UICommandRange = undefined,
command_buffer: [COMMANDLIST_SIZE]u8 = undefined,

pub fn init(
    text_width: ?*const text_width_callback,
    text_height: ?*const text_height_callback,
) Self {
    return Self{
        .text_width = text_width,
        .text_height = text_height,
    };
}

pub fn deinit(self: Self) void {
    _ = self;
}

pub fn begin(self: *Self) void {
    std.debug.assert(self.text_width != null and self.text_height != null);
    self.command_drawer.begin();
    //   self.scroll_target = null;
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

pub fn begin_window(self: *Self, title: []const u8, rect: Rect, opt: ContainerManager.OPT) ?RES {
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

    self.container.begin_root_container(cnt, self.command_drawer.size(), self.input.mouse_pos);
    // clipping is reset here in case a root-container is made within
    // another root-containers's begin/end block; this prevents the inner
    // root-container being clipped to the outer */
    self.command_drawer.push_unclipped_rect();

    // const body = cnt.rect;
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

        //     // do title text
        //     {
        //       mu_Id id = ctx._hash.create("!title", 6);
        //       auto mouseover = ctx.mouse_over(tr);
        //       ctx._input.update_focus_hover(id, tr, opt, mouseover);
        //       ctx._command_drawer.draw_control_text(title, tr, MU_STYLE_TITLETEXT,
        //                                              opt);
        //       if (ctx._input.has_focus(id) &&
        //           ctx._input.mouse_down() == MU_MOUSE_LEFT) {
        //         cnt.rect.x += ctx._input.mouse_delta().x;
        //         cnt.rect.y += ctx._input.mouse_delta().y;
        //       }
        //       body.y += tr.h;
        //       body.h -= tr.h;
        //     }

        //     // do `close` button
        //     if (~opt & MU_OPT_NOCLOSE) {
        //       mu_Id id = ctx._hash.create("!close", 6);
        //       UIRect r = UIRect(tr.x + tr.w - tr.h, tr.y, tr.h, tr.h);
        //       tr.w -= r.w;
        //       ctx._command_drawer.draw_icon(ctx, MU_ICON_CLOSE, r, MU_STYLE_TITLETEXT);
        //       auto mouseover = ctx.mouse_over(r);
        //       ctx._input.update_focus_hover(id, r, opt, mouseover);
        //       if (ctx._input.mouse_pressed() == MU_MOUSE_LEFT &&
        //           ctx._input.has_focus(id)) {
        //         cnt.open = false;
        //       }
        //     }
    }

    //   push_container_body(ctx, cnt, body, opt);

    //   // do `resize` handle
    //   if (~opt & MU_OPT_NORESIZE) {
    //     auto style = ctx._command_drawer.style();
    //     int sz = style.title_height;
    //     mu_Id id = ctx._hash.create("!resize", 7);
    //     UIRect r = UIRect(rect.x + rect.w - sz, rect.y + rect.h - sz, sz, sz);
    //     auto mouseover = ctx.mouse_over(r);
    //     ctx._input.update_focus_hover(id, r, opt, mouseover);
    //     if (ctx._input.has_focus(id) &&
    //         ctx._input.mouse_down() == MU_MOUSE_LEFT) {
    //       cnt.rect.w = mu_max(96, cnt.rect.w + ctx._input.mouse_delta().x);
    //       cnt.rect.h = mu_max(64, cnt.rect.h + ctx._input.mouse_delta().y);
    //     }
    //   }

    //   // resize to content size
    //   if (opt & MU_OPT_AUTOSIZE) {
    //     UIRect r = ctx._layout.back().body;
    //     cnt.rect.w = cnt.content_size.x + (cnt.rect.w - r.w);
    //     cnt.rect.h = cnt.content_size.y + (cnt.rect.h - r.h);
    //   }

    //   // close if this is a popup window and elsewhere was clicked
    //   if (opt & MU_OPT_POPUP && ctx._input.mouse_pressed()) {
    //     if (!ctx._container.is_hover_root(cnt)) {
    //       cnt.open = false;
    //     }
    //   }

    //   ctx._command_drawer.push_clip(cnt.body);
    return .ACTIVE;
}

pub fn end_window(self: *Self) void {
    self.command_drawer.pop_clip();
    // push tail 'goto' jump command and set head 'skip' command. the final steps
    // on initing these are done in mu_end()
    const cnt = self.container.current_container();
    cnt.tail = self.command_drawer.size();
    // pop base clip rect and container
    // self.command_drawer.pop_clip();
    // const layout = self.layout.pop();
    // // auto cnt = self.container.current_container();
    // cnt.content_size.x = layout.max.x - layout.body.x;
    // cnt.content_size.y = layout.max.y - layout.body.y;
    self.container.pop();
    self.hash.pop();
}
