const std = @import("std");
const Rect = @import("./Rect.zig");
const Stack = @import("./stack.zig").Stack;
const Layout = @import("./Layout.zig");
const Style = @import("./Style.zig");

const LAYOUTSTACK_SIZE = 16;

const Self = @This();

stack: Stack(Layout, LAYOUTSTACK_SIZE) = .{},

pub fn end(self: Self) void {
    std.debug.assert(self.stack.size() == 0);
}

pub fn push(self: *Self, layout: Layout) void {
    self.stack.push(layout);
}
pub fn back(self: *Self) *Layout {
    return self.stack.back();
}
pub fn pop(self: *Self) *Layout {
    const b = self.stack.back();
    self.stack.pop();
    return b;
}

pub fn begin_column(self: *Self, style: *const Style) *Layout {
    const b = self.stack.back();
    self.stack.push(Layout.fromRect(b.next(style)));
    return self.stack.back();
}

pub fn end_column(self: *Self) void {
    const b = self.stack.back();
    self.stack.pop();
    // inherit position/next_row/max from child layout if they are greater
    var a = self.stack.back();
    a.inherit_column(b);
}
