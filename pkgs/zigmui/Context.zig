const std = @import("std");
const c = @import("c");
const Rect = @import("./Rect.zig");
const Hash = @import("./Hash.zig");
const OPT = enum(u32) {
    NONE = 0,
    ALIGNCENTER = (1 << 0),
    ALIGNRIGHT = (1 << 1),
    NOINTERACT = (1 << 2),
    NOFRAME = (1 << 3),
    NORESIZE = (1 << 4),
    NOSCROLL = (1 << 5),
    NOCLOSE = (1 << 6),
    NOTITLE = (1 << 7),
    HOLDFOCUS = (1 << 8),
    AUTOSIZE = (1 << 9),
    POPUP = (1 << 10),
    CLOSED = (1 << 11),
    EXPANDED = (1 << 12),
};
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

hash: Hash = .{},
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

pub fn begin(self: Self) void {
    _ = self;
}

pub fn end(self: *Self, frame: *c.UIRenderFrame) !void {
    var fixed = std.heap.FixedBufferAllocator.init(&self.command_buffer);
    var buffer = std.ArrayList(u8).init(fixed.allocator());
    const w = buffer.writer();

    try w.writeIntNative(u32, c.UI_COMMAND_RECT);
    // c.UIRectCommand x, y, w, h, rgba 20bit
    try w.writeIntNative(i32, 10);
    try w.writeIntNative(i32, 10);
    try w.writeIntNative(i32, 10);
    try w.writeIntNative(i32, 10);
    try w.writeIntNative(u8, 255);
    try w.writeIntNative(u8, 255);
    try w.writeIntNative(u8, 255);
    try w.writeIntNative(u8, 255);
    self.command_groups[0].head = 0;
    self.command_groups[0].tail = @intCast(u32, buffer.items.len);

    frame.command_groups = &self.command_groups[0];
    frame.command_group_count = 1;
    frame.command_buffer = &self.command_buffer[0];
}

pub fn begin_window(self: *Self, title: []const u8, rect: Rect, opt: OPT) ?RES {
    _ = rect;
    const id = self.hash.create(title);
    const cnt = self.get_container(id, opt) orelse {
        return null;
    };
    if (!cnt.open) {
        return null;
    }
    ctx.hash.push(id);

    // if (cnt->rect.w == 0) {
    //   cnt->rect = rect;
    // }
    // begin_root_container(ctx, cnt);
    // auto body = cnt->rect;
    // rect = body;

    // // draw frame
    // if (~opt & MU_OPT_NOFRAME) {
    //   ctx->draw_frame(ctx, rect, MU_STYLE_WINDOWBG);
    // }

    // // do title bar
    // if (~opt & MU_OPT_NOTITLE) {
    //   UIRect tr = rect;
    //   tr.h = ctx->style->title_height;
    //   ctx->draw_frame(ctx, tr, MU_STYLE_TITLEBG);

    //   // do title text
    //   if (~opt & MU_OPT_NOTITLE) {
    //     mu_Id id = mu_get_id(ctx, "!title", 6);
    //     mu_update_control(ctx, id, tr, opt);
    //     mu_draw_control_text(ctx, title, tr, MU_STYLE_TITLETEXT, opt);
    //     if (ctx->has_focus(id) && ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
    //       cnt->rect.x += ctx->_input.mouse_delta().x;
    //       cnt->rect.y += ctx->_input.mouse_delta().y;
    //     }
    //     body.y += tr.h;
    //     body.h -= tr.h;
    //   }

    //   // do `close` button
    //   if (~opt & MU_OPT_NOCLOSE) {
    //     mu_Id id = mu_get_id(ctx, "!close", 6);
    //     UIRect r = UIRect(tr.x + tr.w - tr.h, tr.y, tr.h, tr.h);
    //     tr.w -= r.w;
    //     mu_draw_icon(ctx, MU_ICON_CLOSE, r,
    //                  ctx->style->colors[MU_STYLE_TITLETEXT]);
    //     mu_update_control(ctx, id, r, opt);
    //     if (ctx->_input.mouse_pressed() == MU_MOUSE_LEFT && ctx->has_focus(id)) {
    //       cnt->open = false;
    //     }
    //   }
    // }

    // push_container_body(ctx, cnt, body, opt);

    // // do `resize` handle
    // if (~opt & MU_OPT_NORESIZE) {
    //   int sz = ctx->style->title_height;
    //   mu_Id id = mu_get_id(ctx, "!resize", 7);
    //   UIRect r = UIRect(rect.x + rect.w - sz, rect.y + rect.h - sz, sz, sz);
    //   mu_update_control(ctx, id, r, opt);
    //   if (ctx->has_focus(id) && ctx->_input.mouse_down() == MU_MOUSE_LEFT) {
    //     cnt->rect.w = mu_max(96, cnt->rect.w + ctx->_input.mouse_delta().x);
    //     cnt->rect.h = mu_max(64, cnt->rect.h + ctx->_input.mouse_delta().y);
    //   }
    // }

    // // resize to content size
    // if (opt & MU_OPT_AUTOSIZE) {
    //   UIRect r = ctx->layout_stack.back().body;
    //   cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
    //   cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
    // }

    // // close if this is a popup window and elsewhere was clicked
    // if (opt & MU_OPT_POPUP && ctx->_input.mouse_pressed() &&
    //     ctx->hover_root != cnt) {
    //   cnt->open = 0;
    // }

    // ctx->push_clip_rect(cnt->body);
    return .ACTIVE;
}

pub fn end_window(self: Self) void {
    _ = self;
}
