const std = @import("std");
const c = @import("c");
const Self = @This();

id: c.GLuint = undefined,

pub fn init(width: u32, height: u32, grayscale: []const u8) Self {
    var self = Self{};
    c.glGenTextures(1, &self.id);
    c.glBindTexture(c.GL_TEXTURE_2D, self.id);
    c.glTexImage2D(
        c.GL_TEXTURE_2D,
        0,
        c.GL_ALPHA,
        @intCast(c_int, width),
        @intCast(c_int, height),
        0,
        c.GL_ALPHA,
        c.GL_UNSIGNED_BYTE,
        &grayscale[0],
    );
    c.glTexParameteri(c.GL_TEXTURE_2D, c.GL_TEXTURE_MIN_FILTER, c.GL_NEAREST);
    c.glTexParameteri(c.GL_TEXTURE_2D, c.GL_TEXTURE_MAG_FILTER, c.GL_NEAREST);
    std.debug.assert(c.glGetError() == 0);
    return self;
}

pub fn deinit(self: Self) void {
    c.glDeleteTextures(1, &self.id);
}
