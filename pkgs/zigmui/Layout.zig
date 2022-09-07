const std = @import("std");
const Rect = @import("./Rect.zig");
const Vec2 = @import("./Vec2.zig");
const Style = @import("./Style.zig");

const MAX_WIDTHS = 16;

// pub const LAYOUT_TYPE = enum {
//     NONE,
//     RELATIVE,
//     ABSOLUTE,
// };

const Self = @This();

body: Rect = .{},
position: Vec2 = .{},

height: i32 = 0,
width: i32 = 0,
widths: [MAX_WIDTHS]i32 = undefined,
items: usize = 0,
item_index: u32 = 0,

max: Vec2 = .{},

indent: i32 = 0,
next_row: i32 = 0,

// next_type: LAYOUT_TYPE = .NONE,
// next: Rect = .{},

pub fn init() Self {
    var self = Self{};
    self.row(&.{0}, 0);
    return self;
}

pub fn fromRect(rect: Rect) Self {
    var self = init();
    self.body = rect;
    self.max = .{ .x = -0x1000000, .y = -0x1000000 };
    return self;
}

pub fn row(self: *Self, widths: []const i32, height: i32) void {
    self.new_row();
    for (widths) |x, i| {
        self.widths[i] = x;
    }
    self.items = widths.len;
    self.height = height;
}

pub fn new_row(self: *Self) void {
    self.position = .{ .x = self.indent, .y = self.next_row };
    self.item_index = 0;
}

pub fn remain(self: Self) Vec2 {
    return .{
        .x = self.max.x - self.body.x,
        .y = self.max.y - self.body.y,
    };
}
pub fn body_size(self: Self) Vec2 {
    return .{
        .x = self.body.w,
        .y = self.body.h,
    };
}

pub fn inherit_column(self: *Self, b: *const Self) void {
    self.position.x =
        std.math.max(self.position.x, b.position.x + b.body.x - self.body.x);
    self.next_row =
        std.math.max(self.next_row, b.next_row + b.body.y - self.body.y);
    self.max.x = std.math.max(self.max.x, b.max.x);
    self.max.y = std.math.max(self.max.y, b.max.y);
}

pub fn next(self: *Self, style: *const Style) Rect {
    // mu_Layout *layout = &_layout_stack.back();

    var res: Rect = undefined;
    // if (next_type) {
    //   // handle rect set by `mu_layout_set_next`
    //   int type = next_type;
    //   next_type = LAYOUT_NONE;
    //   res = next;
    //   if (type == LAYOUT_ABSOLUTE) {
    //     return res;
    //   }
    // } else
    {
        // handle next row
        if (self.item_index == self.items) {
            // end of row. next row.
            self.new_row();
        }

        // position
        res.x = self.position.x;
        res.y = self.position.y;

        // size
        res.w = if (self.items > 0) self.widths[self.item_index] else self.width;
        res.h = self.height;
        if (res.w == 0) {
            res.w = style.size.x + style.padding * 2;
        }
        if (res.h == 0) {
            res.h = style.size.y + style.padding * 2;
        }
        if (res.w < 0) {
            res.w += self.body.w - res.x + 1;
        }
        if (res.h < 0) {
            res.h += self.body.h - res.y + 1;
        }

        self.item_index += 1;
    }

    // update position
    self.position.x += res.w + style.spacing;
    self.next_row = std.math.max(self.next_row, res.y + res.h + style.spacing);

    // apply body offset
    res.x += self.body.x;
    res.y += self.body.y;

    // update max position
    self.max.x = std.math.max(self.max.x, res.x + res.w);
    self.max.y = std.math.max(self.max.y, res.y + res.h);

    return res;
}
