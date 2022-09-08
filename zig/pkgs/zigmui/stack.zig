const std = @import("std");

pub fn Stack(comptime T: type, comptime N: usize) type {
    return struct {
        const Self = @This();
        items: [N]T = undefined,
        idx: usize = 0,

        pub fn begin(self: *Self) [*]T {
            return @ptrCast([*]T, &self.items[0]);
        }

        pub fn end(self: *Self) [*]T {
            return @ptrCast([*]T, &self.items[self.idx]);
        }

        pub fn size(self: Self) usize {
            return self.idx;
        }

        pub fn get(self: *Self, i: usize) *T {
            return &self.items[i];
        }

        pub fn get_const(self: Self, i: usize) *const T {
            return &self.items[i];
        }

        pub fn grow(self: *Self, n: usize) void {
            std.debug.assert(self.idx + n < N);
            self.idx += n;
        }

        pub fn back(self: *Self) *T {
            return &self.items[self.idx - 1];
        }

        pub fn back_const(self: Self) T {
            return self.items[self.idx - 1];
        }

        pub fn push(self: *Self, val: T) void {
            self.items[self.idx] = val;
            self.idx += 1;
        }

        pub fn pop(self: *Self) void {
            std.debug.assert(self.idx > 0);
            self.idx -= 1;
        }

        pub fn clear(self: *Self) void {
            self.idx = 0;
        }

        pub fn slice(self: *Self) []T {
            return self.items[0..self.idx];
        }

        pub fn slice_const(self: Self) []const T {
            return self.items[0..self.idx];
        }
    };
}
