const c = @import("c");
const Hash = @import("./Hash.zig");
const MU_MAX_FMT = 127;
const MU_REAL_FMT = "%.3g";

const Self = @This();

_buffer: [MU_MAX_FMT]u8 = undefined,
_id: Hash.Id = 0,
_len: c_int = 0,

pub fn set_value(self: *Self, id: Hash.Id, value: f32) void {
    self._id = id;
    self._len = c.sprintf(&self._buffer[0], MU_REAL_FMT, value);
}

pub fn buffer(self: *Self, id: Hash.Id) ?[]u8 {
    if (id != self._id) {
        return null;
    }
    return self._buffer[0..@intCast(usize, self._len)];
}

pub fn commit(self: *Self) f32 {
    self._id = 0;
    return @floatCast(f32, c.strtod(&self._buffer[0], null));
}
