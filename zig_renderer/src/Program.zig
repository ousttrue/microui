const std = @import("std");
const c = @import("c");
const logger = std.log.scoped(.Program);
const Self = @This();

id: u32 = 0,

fn compile_shader(shader_type: c.GLenum, src: []const u8) !u32 {
    const id = c.glCreateShader(shader_type);
    errdefer c.glDeleteShader(id);
    const len = @intCast(c_int, src.len);
    const pSrc = &src[0];
    c.glShaderSource(id, 1, &pSrc, &len);
    c.glCompileShader(id);
    var vertex_compiled: c.GLint = undefined;
    c.glGetShaderiv(id, c.GL_COMPILE_STATUS, &vertex_compiled);
    if (vertex_compiled == c.GL_TRUE) {
        return id;
    }
    var log_length: c.GLsizei = 0;
    var log_buf: [1024]u8 = undefined;
    c.glGetShaderInfoLog(id, log_buf.len, &log_length, &log_buf[0]);
    if (log_length > 0) {
        logger.err("{s}", .{log_buf[0..@intCast(usize, log_length)]});
    }
    return error.compileError;
}

pub fn create(vs: []const u8, fs: []const u8) ?Self {
    const vs_id = compile_shader(c.GL_VERTEX_SHADER, vs) catch {
        return null;
    };
    defer c.glDeleteShader(vs_id);
    const fs_id = compile_shader(c.GL_FRAGMENT_SHADER, fs) catch {
        return null;
    };
    defer c.glDeleteShader(fs_id);

    var self = Self{
        .id = c.glCreateProgram(),
    };
    c.glAttachShader(self.id, vs_id);
    c.glAttachShader(self.id, fs_id);
    c.glLinkProgram(self.id);

    return self;
}

pub fn bind(self: Self) void {
    c.glUseProgram(self.id);
}
pub fn unbind(_: Self) void {
    c.glUseProgram(0);
}
pub fn set_uniform_matrix(
    self: Self,
    key: [:0]const u8,
    m: *const f32,
    transpose: bool,
) void {
    const location = c.glGetUniformLocation(self.id, &key[0]);
    c.glUniformMatrix4fv(location, 1, if (transpose) 1 else 0, m);
}
