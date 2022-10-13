const std = @import("std");
const builtin = @import("builtin");
const zigmui = @import("zigmui");
const gl = @import("gl");
const Texture = @import("./Texture.zig");
const Program = @import("./Program.zig");
const VS = @embedFile("./quad.vs");
const FS = @embedFile("./quad.fs");
const Vbo = @import("./Vbo.zig");
const Ibo = @import("./Ibo.zig");
const Vao = @import("./Vao.zig");
const atlas = @import("atlas");
const MAX_QUADS = 12800;

const Vertex = struct {
    x: f32,
    y: f32,
    u: f32,
    v: f32,
    r: u8,
    g: u8,
    b: u8,
    a: u8,
};

const Self = @This();

width: i32 = 1,
height: i32 = 1,
matrix: [16]f32 = .{
    1, 0, 0, 0, //
    0, 1, 0, 0, //
    0, 0, 1, 0, //
    0, 0, 0, 1, //
},
atlas_texture: Texture,
program: Program,
vao: Vao,
vertices: [MAX_QUADS * 4]Vertex = undefined,
vertex_count: u32 = 0,
indices: [MAX_QUADS * 6]u32 = undefined,
index_count: u32 = 0,

// init OpenGL by glad
const GLADloadproc = fn ([*c]const u8) callconv(.C) ?*anyopaque;
pub extern fn gladLoadGL(*const GLADloadproc) c_int;
pub fn loadproc(ptr: *const anyopaque) void {
    if (builtin.target.cpu.arch != .wasm32) {
        _ = gladLoadGL(@ptrCast(*const GLADloadproc, ptr));
    }
}

pub fn init(p: *const anyopaque, atlas_width: u32, atlas_height: u32, atlas_data: []const u8) Self {
    if (builtin.target.cpu.arch != .wasm32) {
        _ = gladLoadGL(@ptrCast(*const GLADloadproc, @alignCast(@alignOf(GLADloadproc), p)));
    }

    const vbo = Vbo.init(MAX_QUADS * 6 * @sizeOf(Vertex));
    const ibo = Ibo.init(MAX_QUADS * 4 * @sizeOf(Vertex));
    var self = Self{
        .atlas_texture = Texture.init(atlas_width, atlas_height, atlas_data),
        .program = Program.create(VS, FS) orelse unreachable,
        .vao = Vao.init(vbo, ibo),
    };

    // https://www.khronos.org/opengl/wiki/Vertex_Specification
    self.vao.bind();
    vbo.bind();
    gl.enableVertexAttribArray(0);
    gl.vertexAttribPointer(0, 2, gl.GL_FLOAT, gl.GL_FALSE, @sizeOf(Vertex), 0);
    gl.enableVertexAttribArray(1);
    gl.vertexAttribPointer(1, 2, gl.GL_FLOAT, gl.GL_FALSE, @sizeOf(Vertex), @offsetOf(Vertex, "u"));
    gl.enableVertexAttribArray(2);
    gl.vertexAttribPointer(2, 4, gl.GL_UNSIGNED_BYTE, gl.GL_TRUE, @sizeOf(Vertex), @offsetOf(Vertex, "r"));
    ibo.bind();

    self.vao.unbind();
    vbo.unbind();
    ibo.unbind();

    return self;
}

pub fn begin(self: *Self, width: i32, height: i32, bg: []const f32) void {
    self.width = width;
    self.height = height;
    gl.viewport(0, 0, width, height);
    gl.scissor(0, 0, width, height);
    gl.clearColor(
        bg[0] / 255.0,
        bg[1] / 255.0,
        bg[2] / 255.0,
        1,
    );
    gl.clear(gl.GL_COLOR_BUFFER_BIT);

    self.matrix[0] = 2.0 / @intToFloat(f32, width);
    self.matrix[3] = -1.0;
    self.matrix[5] = 2.0 / -@intToFloat(f32, height);
    self.matrix[7] = 1.0;
}

pub fn flush(self: *Self) void {
    if (self.vertex_count > 0 and self.index_count > 0) {
        // update
        self.vao.vbo.update(&self.vertices[0], @sizeOf(Vertex) * self.vertex_count, self.vertex_count);
        self.vertex_count = 0;
        self.vao.ibo.update(&self.indices[0], @sizeOf(u32) * self.index_count, self.index_count);
        self.index_count = 0;

        gl.disable(gl.GL_CULL_FACE);
        gl.disable(gl.GL_DEPTH_TEST);
        gl.enable(gl.GL_SCISSOR_TEST);
        // gl.enable(gl.GL_TEXTURE_2D); not exists in WebGL

        // alpha blend
        gl.enable(gl.GL_BLEND);
        gl.blendFunc(gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA);

        // draw
        self.program.bind();
        self.program.set_uniform_matrix("M", &self.matrix[0], true);
        gl.activeTexture(gl.GL_TEXTURE0);
        self.atlas_texture.bind();
        self.vao.draw();
        self.atlas_texture.unbind();
        self.program.unbind();
    }
}

pub fn draw_rect(self: *Self, rect: zigmui.Rect, color: zigmui.Color32) void {
    self.push_quad(rect, atlas.atlas[@enumToInt(atlas.ATLAS_GLYPH.ATLAS_WHITE)], color);
}

pub fn draw_text(self: *Self, text: []const u8, pos: zigmui.Vec2, color: zigmui.Color32) void {
    var x = pos.x;
    for (text) |ch| {
        if ((ch & 0xc0) == 0x80) {
            continue;
        }
        const chr = std.math.min(ch, 127);
        const glyph = atlas.atlas[chr];
        self.push_quad(.{
            .x = x,
            .y = pos.y,
            .w = glyph[2],
            .h = glyph[3],
        }, glyph, color);
        x += glyph[2];
    }
}

pub fn draw_icon(self: *Self, id: u32, rect: zigmui.Rect, color: zigmui.Color32) void {
    const glyph = atlas.atlas[id];
    const x = rect.x + @divTrunc(rect.w - glyph[2], 2);
    const y = rect.y + @divTrunc(rect.h - glyph[3], 2);
    self.push_quad(.{ .x = x, .y = y, .w = glyph[2], .h = glyph[3] }, glyph, color);
}

pub fn set_clip_rect(self: *Self, rect: zigmui.Rect) void {
    self.flush();
    gl.scissor(rect.x, self.height - (rect.y + rect.h), rect.w, rect.h);
}

fn push_quad(self: *Self, quad: zigmui.Rect, glyph: atlas.Rect, color: zigmui.Color32) void {
    const x = @intToFloat(f32, glyph[0]) / @intToFloat(f32, atlas.width);
    const y = @intToFloat(f32, glyph[1]) / @intToFloat(f32, atlas.height);
    const w = @intToFloat(f32, glyph[2]) / @intToFloat(f32, atlas.width);
    const h = @intToFloat(f32, glyph[3]) / @intToFloat(f32, atlas.height);

    const i = self.vertex_count;
    self.push_triangle(i + 0, i + 1, i + 2);
    self.push_triangle(i + 2, i + 3, i + 1);

    // 0 - 1
    // |   |
    // 2 - 3
    //
    // 0,1,2
    // 2,3,1

    self.vertices[self.vertex_count] = Vertex{
        .x = @intToFloat(f32, quad.x),
        .y = @intToFloat(f32, quad.y),
        .u = x,
        .v = y,
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a,
    };
    self.vertex_count += 1;

    self.vertices[self.vertex_count] = Vertex{
        .x = @intToFloat(f32, quad.x + quad.w),
        .y = @intToFloat(f32, quad.y),
        .u = x + w,
        .v = y,
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a,
    };
    self.vertex_count += 1;

    self.vertices[self.vertex_count] = Vertex{
        .x = @intToFloat(f32, quad.x),
        .y = @intToFloat(f32, quad.y + quad.h),
        .u = x,
        .v = y + h,
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a,
    };
    self.vertex_count += 1;

    self.vertices[self.vertex_count] = Vertex{
        .x = @intToFloat(f32, quad.x + quad.w),
        .y = @intToFloat(f32, quad.y + quad.h),
        .u = x + w,
        .v = y + h,
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a,
    };
    self.vertex_count += 1;
}

fn push_triangle(self: *Self, idx0: u32, idx1: u32, idx2: u32) void {
    self.indices[self.index_count] = idx0;
    self.index_count += 1;
    self.indices[self.index_count] = idx1;
    self.index_count += 1;
    self.indices[self.index_count] = idx2;
    self.index_count += 1;
}
