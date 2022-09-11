const std = @import("std");
const builtin = @import("builtin");
pub const CommandRange = extern struct {
    head: c_uint,
    tail: c_uint,
};

const Self = @This();

command_groups: *const CommandRange,
command_group_count: c_uint,
command_buffer: *const u8,

// comptime {
//     if (builtin.target.cpu.arch == .wasm32) {
//         std.debug.assert(@SizeOf(Self) == 12);
//     }
// }

pub fn slice(self: Self) []const CommandRange {
    return @ptrCast([*]const CommandRange, self.command_groups)[0..self.command_group_count];
}

pub fn get(self: Self, idx: u32) [*]const u8 {
    return @ptrCast([*]const u8, self.command_buffer) + idx;
}
