const Rect = @import("./Rect.zig");
const Vec2 = @import("./Vec2.zig");

const MAX_WIDTHS = 16;

pub const LAYOUT_TYPE = enum {
    NONE,
    RELATIVE,
    ABSOLUTE,
};

const Self = @This();

body: Rect = .{},
next: Rect = .{},
position: Vec2 = .{},
size: Vec2 = .{},
max: Vec2 = .{},
widths: [MAX_WIDTHS]i32 = undefined,
items: usize = 0,
item_index: i32 = 0,
next_row: i32 = 0,
next_type: LAYOUT_TYPE = .NONE,
indent: i32 = 0,

pub fn init(widths: []const i32) Self {
    var self = Self{};
    self.row(widths, 0);
    return self;
}

pub fn create(body: Rect, scroll: Vec2) Self {
    var self = init(&.{0});
    self.body = .{ .x = body.x - scroll.x, .y = body.y - scroll.y, .w = body.w, .h = body.h };
    self.max = .{ .x = -0x1000000, .y = -0x1000000 };
    return self;
}

pub fn row(self: *Self, widths: []const i32, height: i32) void {
    for (widths) |x, i| {
        self.widths[i] = x;
    }
    self.items = widths.len;
    self.position = .{ .x = self.indent, .y = self.next_row };
    self.size.y = height;
    self.item_index = 0;
}
