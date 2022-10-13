const std = @import("std");
const gl = @import("gl");
const logger = std.log.scoped(.Scene);
const Self = @This();

pub fn init() Self {
    return Self{};
}

pub fn load(self: *Self, path: []const u8) void {
    _ = self;
    logger.debug("load: {s}", .{path});
}

pub fn clear(self: *Self, width: i32, height: i32, bg: []const f32) void {
    _ = self;
    gl.viewport(0, 0, width, height);
    gl.scissor(0, 0, width, height);
    gl.clearColor(
        bg[0] / 255.0,
        bg[1] / 255.0,
        bg[2] / 255.0,
        1,
    );
    gl.clear(gl.GL_COLOR_BUFFER_BIT);
}
