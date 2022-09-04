const c = @import("c");
const Stack = @import("./stack.zig").Stack;
const Rect = @import("./Rect.zig");
const Vec2 = @import("./Vec2.zig");
const Color32 = @import("./Color32.zig");
const ClipStack = @import("./ClipStack.zig");
const Style = @import("./Style.zig");
const COMMANDLIST_SIZE = (256 * 1024);

pub const COMMAND = enum(u32) {
    CLIP,
    RECT,
    TEXT,
    ICON,
};

const Self = @This();

command_list: Stack(u8, COMMANDLIST_SIZE) = .{},
clip_stack: ClipStack = .{},
style: Style = .{},

pub fn begin(self: *Self) void {
    self.command_list.clear();
}

pub fn end(self: *Self, command: *c.UIRenderFrame) void {
    self.clip_stack.end();
    command.command_buffer = @ptrCast([*c]u8, self.command_list.get(0));
}

pub fn push_clip(self: *Self, r: Rect) void {
    self.clip_stack.push(r);
}
pub fn push_unclipped_rect(self: *Self) void {
    self.clip_stack.push_unclipped_rect();
}
pub fn pop_clip(self: *Self) void {
    self.clip_stack.pop();
}
pub fn overlaps_clip(self: Self, pos: Vec2) bool {
    return self.clip_stack.back().overlaps_vec2(pos);
}

pub fn size(self: Self) usize {
    return self.command_list.size();
}

// pub fn get(self: *Self, i: usize) [*]const u8 {
//     return @ptrCast([*]const u8, self.command_list.get(i));
// }

pub fn push_command(self: *Self, command: u32, text_length: usize) [*]u8 {
    const cmd = @ptrCast([*]u8, self.command_list.end());
    @memcpy(cmd, @ptrCast([*]const u8, &command), @sizeOf(u32));
    const command_size = switch (command) {
        c.UI_COMMAND_CLIP => @sizeOf(c.UIClipCommand),
        c.UI_COMMAND_RECT => @sizeOf(c.UIRectCommand),
        c.UI_COMMAND_TEXT => blk: {
            var text = @ptrCast(*c.UITextCommand, @alignCast(@alignOf(c.UITextCommand), cmd + 4));
            text.length = @intCast(u32, text_length);
            break :blk @sizeOf(c.UITextCommand) + text_length;
        },
        c.UI_COMMAND_ICON => @sizeOf(c.UIIconCommand),
        else => unreachable,
    };
    self.command_list.grow(4 + command_size);
    return cmd;
}

//   void set_clip(const UIRect &rect) {
//     auto cmd = push_command(UI_COMMAND_CLIP);
//     cmd->clip()->rect = rect;
//   }

//   void push_text(const char *str, int len, const UIVec2 &pos,
//                  const UIColor32 &color, const void *font) {
//     if (len < 0) {
//       len = strlen(str);
//     }
//     auto cmd = push_command(UI_COMMAND_TEXT, len);
//     memcpy((char *)cmd->text()->begin(), str, len);
//     cmd->text()->pos = pos;
//     cmd->text()->color = color;
//     cmd->text()->font = font;
//   }

//   void push_icon(int id, const UIRect &rect, const UIColor32 &color) {
//     auto cmd = push_command(UI_COMMAND_ICON);
//     cmd->icon()->id = id;
//     cmd->icon()->rect = rect;
//     cmd->icon()->color = color;
//   }

pub fn push_rect(self: *Self, rect: Rect, color: Color32) void {
    var p = self.push_command(c.UI_COMMAND_RECT, 0);
    var cmd = @ptrCast(*c.UIRectCommand, @alignCast(@alignOf(c.UIRectCommand), p + 4));
    cmd.rect.x = rect.x;
    cmd.rect.y = rect.y;
    cmd.rect.w = rect.w;
    cmd.rect.h = rect.h;
    cmd.color.r = color.r;
    cmd.color.g = color.g;
    cmd.color.b = color.b;
    cmd.color.a = color.a;
}

pub fn draw_rect(self: *Self, rect: Rect, color: Color32) void {
    self.push_rect(self.clip_stack.intersect(rect), color);
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
