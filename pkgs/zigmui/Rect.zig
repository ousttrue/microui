const std = @import("std");
const Vec2 = @import("./Vec2.zig");
const Self = @This();

x: i32 = 0,
y: i32 = 0,
w: i32 = 0,
h: i32 = 0,

pub const UNCLIPPED_RECT = Self{ .x = 0, .y = 0, .w = 0x1000000, .h = 0x1000000 };

pub fn expand(self: Self, n: i32) Self {
    return Self{
        .x = self.x - n,
        .y = self.y - n,
        .w = self.w + n * 2,
        .h = self.h + n * 2,
    };
}

pub fn intersect(self: Self, r2: Self) Self {
    var x1 = std.math.max(self.x, r2.x);
    var y1 = std.math.max(self.y, r2.y);
    var x2 = std.math.min(self.x + self.w, r2.x + r2.w);
    var y2 = std.math.min(self.y + self.h, r2.y + r2.h);
    if (x2 < x1) {
        x2 = x1;
    }
    if (y2 < y1) {
        y2 = y1;
    }
    return Self{ .x = x1, .y = y1, .w = x2 - x1, .h = y2 - y1 };
}

pub fn overlaps_vec2(self: Self, p: Vec2) bool {
    return p.x >= self.x and p.x < self.x + self.w and p.y >= self.y and
        p.y < self.y + self.h;
}

pub fn sub(self: Self, p: Vec2) Self {
    return .{
        .x = self.x - p.x,
        .y = self.y - p.y,
        .w = self.w,
        .h = self.h,
    };
}
