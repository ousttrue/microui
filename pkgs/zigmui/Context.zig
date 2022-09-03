const std = @import("std");
const c = @import("c");
const Rect = @import("./Rect.zig");
const Color32 = @import("./Color32.zig");
const Hash = @import("./Hash.zig");
const ContainerManager = @import("./ContainerManager.zig");
const CommandStack = @import("./CommandStack.zig");
const ClipStack = @import("./ClipStack.zig");
const Input = @import("./Input.zig");
const Style = @import("./Style.zig");
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
command_stack: CommandStack = .{},
input: Input = .{},
clip_stack: ClipStack = .{},
style: Style = .{},

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
    self.command_stack.begin();
    //   self.scroll_target = null;
    self.container.begin();
    self.input.begin();
    self.frame += 1;
}

pub fn end(self: *Self, command: *c.UIRenderFrame) !void {
    // var fixed = std.heap.FixedBufferAllocator.init(&self.command_buffer);
    // var buffer = std.ArrayList(u8).init(fixed.allocator());
    // const w = buffer.writer();

    // check stacks
    self.clip_stack.end();
    self.hash.end();
    // std.debug.assert(self.layout_stack.size() == 0);

    // handle scroll input
    const mouse_pressed = self.input.mouse_pressed;

    // if (self.scroll_target) {
    //     self.scroll_target.scroll += self._input.scroll_delta();
    // }

    // unset focus if focus id was not touched this frame
    // if (!self.updated_focus) {
    //     self.focus = 0;
    // }
    // self.updated_focus = false;

    // reset input state
    self.input.end();

    self.container.end(mouse_pressed, command);
    command.command_buffer = self.command_stack.get(0);
}

pub fn draw_rect(self: *Self, rect: Rect, color: Color32) void {
    self.command_stack.push_rect(self.clip_stack.intersect(rect), color);
}

pub fn draw_box(self: *Self, rect: Rect, color: Color32) void {
    self.draw_rect(Rect{ .x = rect.x + 1, .y = rect.y, .w = rect.w - 2, .h = 1 }, color);
    self.draw_rect(Rect{ .x = rect.x + 1, .y = rect.y + rect.h - 1, .w = rect.w - 2, .h = 1 }, color);
    self.draw_rect(Rect{ .x = rect.x, .y = rect.y, .w = 1, .h = rect.h }, color);
    self.draw_rect(Rect{ .x = rect.x + rect.w - 1, .y = rect.y, .w = 1, .h = rect.h }, color);
}

pub fn draw_frame(self: *Self, rect: Rect, colorid: Style.STYLE) void {
    self.draw_rect(rect, self.style.colors[@enumToInt(colorid)]);
    if (colorid == .SCROLLBASE or colorid == .SCROLLTHUMB or
        colorid == .TITLEBG)
    {
        return;
    }
    // draw border
    if (self.style.colors[@enumToInt(Style.STYLE.BORDER)].a > 0) {
        self.draw_box(rect.expand(1), self.style.colors[@enumToInt(Style.STYLE.BORDER)]);
    }
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

    self.container.begin_root_container(cnt, self.command_stack.size(), self.input.mouse_pos);
    // clipping is reset here in case a root-container is made within
    // another root-containers's begin/end block; this prevents the inner
    // root-container being clipped to the outer */
    self.clip_stack.push_unclipped_rect();

    // const body = cnt.rect;
    // rect = body;

    // draw frame
    if (!opt.contains(.NOFRAME)) {
        self.draw_frame(cnt.rect, .WINDOWBG);
    }

    // // do title bar
    // if (~opt & MU_OPT_NOTITLE) {
    //   UIRect tr = rect;
    //   tr.h = self.style->title_height;
    //   self.draw_frame(ctx, tr, MU_STYLE_TITLEBG);

    //   // do title text
    //   if (~opt & MU_OPT_NOTITLE) {
    //     mu_Id id = mu_get_id(ctx, "!title", 6);
    //     mu_update_control(ctx, id, tr, opt);
    //     mu_draw_control_text(ctx, title, tr, MU_STYLE_TITLETEXT, opt);
    //     if (self.has_focus(id) && self._input.mouse_down() == MU_MOUSE_LEFT) {
    //       cnt->rect.x += self._input.mouse_delta().x;
    //       cnt->rect.y += self._input.mouse_delta().y;
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
    //                  self.style->colors[MU_STYLE_TITLETEXT]);
    //     mu_update_control(ctx, id, r, opt);
    //     if (self._input.mouse_pressed() == MU_MOUSE_LEFT && self.has_focus(id)) {
    //       cnt->open = false;
    //     }
    //   }
    // }

    // push_container_body(ctx, cnt, body, opt);

    // // do `resize` handle
    // if (~opt & MU_OPT_NORESIZE) {
    //   int sz = self.style->title_height;
    //   mu_Id id = mu_get_id(ctx, "!resize", 7);
    //   UIRect r = UIRect(rect.x + rect.w - sz, rect.y + rect.h - sz, sz, sz);
    //   mu_update_control(ctx, id, r, opt);
    //   if (self.has_focus(id) && self._input.mouse_down() == MU_MOUSE_LEFT) {
    //     cnt->rect.w = mu_max(96, cnt->rect.w + self._input.mouse_delta().x);
    //     cnt->rect.h = mu_max(64, cnt->rect.h + self._input.mouse_delta().y);
    //   }
    // }

    // // resize to content size
    // if (opt & MU_OPT_AUTOSIZE) {
    //   UIRect r = self.layout_stack.back().body;
    //   cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
    //   cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
    // }

    // // close if this is a popup window and elsewhere was clicked
    // if (opt & MU_OPT_POPUP && self._input.mouse_pressed() &&
    //     self.hover_root != cnt) {
    //   cnt->open = 0;
    // }

    // self.push(cnt->body);
    return .ACTIVE;
}

pub fn end_window(self: *Self) void {
    // self.clip_stack.pop();

    // push tail 'goto' jump command and set head 'skip' command. the final steps
    // on initing these are done in mu_end() */
    var cnt = self.container.current_container();
    cnt.tail = self.command_stack.size();
    // pop base clip rect and container
    self.clip_stack.pop();
    self.pop_container();
}

pub fn pop_container(self: *Self) void {
    //   mu_Layout *layout = &self.layout_stack.back();
    //   mu_Container *cnt = self._container.current_container();
    //   cnt->content_size.x = layout->max.x - layout->body.x;
    //   cnt->content_size.y = layout->max.y - layout->body.y;
    self.container.pop();
    // self.layout_stack.pop();
    self.hash.stack.pop();
}
