const std = @import("std");
const Rect = @import("./Rect.zig");
const Vec2 = @import("./Vec2.zig");
const Stack = @import("./stack.zig").Stack;
const CLIP = enum { NONE, PART, ALL };

const CLIPSTACK_SIZE = 32;
const Self = @This();

clip_stack: Stack(Rect, CLIPSTACK_SIZE) = .{},

pub fn end(self: Self) void {
    std.debug.assert(self.clip_stack.size() == 0);
}

pub fn intersect(self: Self, rect: Rect) Rect {
    return rect.intersect(self.clip_stack.back_const());
}

pub fn overlaps(self: Self, pos: Vec2) bool {
    return self.clip_stack.back_const().overlaps_vec2(pos);
}

pub fn push_unclipped_rect(self: *Self) void {
    self.clip_stack.push(Rect.UNCLIPPED_RECT);
}

pub fn push(self: *Self, rect: Rect) void {
    const last = self.clip_stack.back().*;
    self.clip_stack.push(rect.intersect(last));
}

pub fn back(self: *Self) *Rect {
    return self.clip_stack.back();
}

pub fn back_const(self: Self) Rect {
    return self.clip_stack.back_const();
}

pub fn pop(self: *Self) void {
    self.clip_stack.pop();
}

pub fn check_clip(self: Self, r: Rect) CLIP {
    const cr = self.clip_stack.back_const();
    if (r.x > cr.x + cr.w or r.x + r.w < cr.x or r.y > cr.y + cr.h or r.y + r.h < cr.y) {
        return .ALL;
    }
    if (r.x >= cr.x and r.x + r.w <= cr.x + cr.w and r.y >= cr.y and r.y + r.h <= cr.y + cr.h) {
        return .NONE;
    }
    return .PART;
}
