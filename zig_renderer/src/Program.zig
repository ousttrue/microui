const std = @import("std");
const gl = @import("./gl.zig");
const logger = std.log.scoped(.Program);
const Self = @This();

id: u32 = 0,

fn compile_shader(shader_type: gl.GLenum, src: [:0]const u8) !u32 {
    const id = gl.createShader(shader_type);
    errdefer gl.deleteShader(id);
    const pSrc = @ptrCast([*:0]const u8, &src[0]);
    gl.shaderSource(id, 1, &pSrc);
    gl.compileShader(id);
    var vertex_compiled: c_int = undefined;
    gl.getShaderiv(id, gl.GL_COMPILE_STATUS, &vertex_compiled);
    if (vertex_compiled == gl.GL_TRUE) {
        return id;
    }
    var log_length: c_int = 0;
    var log_buf: [1024]u8 = undefined;
    gl.getShaderInfoLog(id, log_buf.len, &log_length, &log_buf[0]);
    if (log_length > 0) {
        logger.err("{s}", .{log_buf[0..@intCast(usize, log_length)]});
    }
    return error.compileError;
}

pub fn create(vs: [:0]const u8, fs: [:0]const u8) ?Self {
    const vs_id = compile_shader(gl.GL_VERTEX_SHADER, vs) catch {
        return null;
    };
    defer gl.deleteShader(vs_id);
    const fs_id = compile_shader(gl.GL_FRAGMENT_SHADER, fs) catch {
        return null;
    };
    defer gl.deleteShader(fs_id);

    var self = Self{
        .id = gl.createProgram(),
    };
    gl.attachShader(self.id, vs_id);
    gl.attachShader(self.id, fs_id);
    gl.linkProgram(self.id);

    return self;
}

pub fn bind(self: Self) void {
    gl.useProgram(self.id);
}
pub fn unbind(_: Self) void {
    gl.useProgram(0);
}
pub fn set_uniform_matrix(
    self: Self,
    key: [:0]const u8,
    m: *const f32,
    transpose: bool,
) void {
    const location = gl.getUniformLocation(self.id, @ptrCast([*:0]const u8, &key[0]));
    gl.uniformMatrix4fv(location, 1, if (transpose) 1 else 0, m);
}
