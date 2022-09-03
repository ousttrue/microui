const c = @import("c");
const Stack = @import("./stack.zig").Stack;
const Rect = @import("./Rect.zig");
const Color32 = @import("./Color32.zig");
const COMMANDLIST_SIZE = (256 * 1024);

pub const COMMAND = enum(u32) {
    CLIP,
    RECT,
    TEXT,
    ICON,
};

const Self = @This();

command_list: Stack(u8, COMMANDLIST_SIZE) = .{},

pub fn begin(self: *Self) void {
    self.command_list.clear();
}

pub fn size(self: Self) usize {
    return self.command_list.size();
}

pub fn get(self: *Self, i: usize) [*]const u8 {
    return @ptrCast([*]const u8, self.command_list.get(i));
}

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
