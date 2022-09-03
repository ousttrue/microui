const std = @import("std");
const Stack = @import("stack.zig").Stack;
const IDSTACK_SIZE = 32;

// 32bit fnv-1a hash
const HASH_INITIAL = 2166136261;

const Id = u32;
fn calc_hash(src: Id, data: []const u8) Id {
    var p = @ptrCast([*]const u8, &data[0]);
    var size = data.len;
    var hash = src;
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

pub fn validate_empty(self: Self) void {
    std.debug.assert(self.stack.size() == 0);
}

pub fn create(self: *Self, data: []const u8) Id {
    const res = if (self.stack.size() > 0) self.stack.back() else HASH_INITIAL;
    self.last = calc_hash(res, data);
    return self.last;
}

pub fn create_push(self: *Self, data: []const u8) Id {
    const id = self.create(data);
    self.stack.push(id);
    return id;
}

pub fn push_last(self: Self) void {
    self.stack.push(self.last);
}
