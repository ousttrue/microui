const Rect = @import("./Rect.zig");
const Vec2 = @import("./Vec2.zig");
const Self = @This();

head: usize = 0,
tail: usize = 0,
rect: Rect = .{},
body: Rect = .{},
content_size: Vec2 = .{},
scroll: Vec2 = .{},
zindex: i32 = 0,
open: bool = true,

pub fn reset(self: *Self) void {
    self.head = 0;
    self.tail = 0;
    self.rect = .{};
    self.body = .{};
    self.content_size = .{};
    self.scroll = .{};
    self.zindex = 0;
    self.open = true;
}
