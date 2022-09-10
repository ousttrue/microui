const std = @import("std");
// const c = @import("c");
const Hash = @import("./Hash.zig");
const MU_MAX_FMT = 127;
const MU_REAL_FMT = "%.3g";

// pub fn sprintf(buf: []u8, fmt: [:0]const u8, args: anytype) []const u8 {
//     const a = .{
//         @ptrCast([*c]u8, &buf[0]),
//         buf.len,
//         @ptrCast([*c]const u8, &fmt[0]),
//     } ++ args;
//     var len = @call(
//         .{},
//         c.snprintf,
//         a,
//     );
//     return buf[0..@intCast(usize, len)];
// }

const Self = @This();

_buffer: [MU_MAX_FMT]u8 = undefined,
_id: Hash.Id = 0,
_len: usize = 0,

pub fn set_value(self: *Self, id: Hash.Id, value: f32) void {
    self._id = id;
    if (std.fmt.bufPrint(&self._buffer, "{}", .{value})) |slice| {
        self._len = slice.len;
    } else |_| {
        self._len = 0;
    }
}

pub fn buffer(self: *Self, id: Hash.Id) ?[]u8 {
    if (id != self._id) {
        return null;
    }
    return self._buffer[0..@intCast(usize, self._len)];
}

pub fn commit(self: *Self) f32 {
    self._id = 0;
    if (std.fmt.parseFloat(f32, self._buffer[0..self._len])) |value| {
        return value;
    } else |_| {
        return 0;
    }
}
