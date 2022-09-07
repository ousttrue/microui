const std = @import("std");
const Stack = @import("stack.zig").Stack;
const IDSTACK_SIZE = 32;

// 32bit fnv-1a hash
const HASH_INITIAL = 2166136261;

pub const Id = u32;
fn calc_hash(src: Id, begin: *const u8, len: usize) Id {
    @setRuntimeSafety(false);
    var hash = src;
    var size = @intCast(i32, len);
    var p = @ptrCast([*]const u8, begin);
    while (size >= 0) : ({
        size -= 1;
        p += 1;
    }) {
        hash = (hash ^ p[0]) * 16777619;
    }
    return hash;
}

const Self = @This();
stack: Stack(Id, IDSTACK_SIZE) = .{},
last: Id = 0,

pub fn end(self: Self) void {
    std.debug.assert(self.stack.size() == 0);
}

pub fn from_str(self: *Self, src: []const u8) Id {
    const res = if (self.stack.size() > 0) self.stack.back_const() else HASH_INITIAL;
    self.last = calc_hash(res, &src[0], src.len);
    return self.last;
}

pub fn from_value(self: *Self, value: anytype) Id {
    const res = if (self.stack.size() > 0) self.stack.back_const() else HASH_INITIAL;
    const T = @TypeOf(value);
    self.last = calc_hash(res, @ptrCast(*const u8, &value), @sizeOf(T));
    return self.last;
}

// pub fn create_push(self: *Self, data: []const u8) Id {
//     const id = self.create(data);
//     self.stack.push(id);
//     return id;
// }

pub fn push_last(self: *Self) void {
    self.stack.push(self.last);
}

// pub fn pop(self: *Self) void {
//     self.stack.pop();
// }
