const Texture = @import("./Texture.zig");
const Self = @This();

atlas_texture: Texture = undefined,

pub fn init() Self {
    return Self{};
}

pub fn loadfunc(self: Self, p: *const anyopaque) void {
    _ = self;
    _ = p;
}

pub fn begin(self: *Self, width: i32, height: i32, bg: []const f32) void {
    _ = self;
    _ = width;
    _ = height;
    _ = bg;
}

pub fn flush(self: Self) void {
    _ = self;
}
