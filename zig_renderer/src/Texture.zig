const std = @import("std");
const Self = @This();
const gl = @import("gl");

id: c_uint = undefined,

pub fn init(width: u32, height: u32, grayscale: []const u8) Self {
    var self = Self{};
    gl.genTextures(1, &self.id);
    gl.bindTexture(gl.GL_TEXTURE_2D, self.id);
    gl.texImage2D(
        gl.GL_TEXTURE_2D,
        0,
        gl.GL_ALPHA,
        @intCast(c_int, width),
        @intCast(c_int, height),
        0,
        gl.GL_ALPHA,
        gl.GL_UNSIGNED_BYTE,
        &grayscale[0],
    );
    gl.texParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_NEAREST);
    gl.texParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_NEAREST);
    // std.debug.assert(gl.getError() == 0);
    return self;
}

pub fn deinit(self: Self) void {
    gl.deleteTextures(1, &self.id);
}

pub fn bind(self: Self) void {
    gl.bindTexture(gl.GL_TEXTURE_2D, self.id);
}
pub fn unbind(_: Self) void {
    gl.bindTexture(gl.GL_TEXTURE_2D, 0);
}
