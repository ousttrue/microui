const c = @import("c");
const Texture = @import("./Texture.zig");
const Program = @import("./Program.zig");
const VS = @embedFile("./quad.vs");
const FS = @embedFile("./quad.fs");
const Vbo = @import("./Vbo.zig");
const Ibo = @import("./Ibo.zig");
const Vao = @import("./Vao.zig");
const atlas = @import("./atlas.zig");
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

pub fn init(p: *const anyopaque, atlas_width: u32, atlas_height: u32, atlas_data: []const u8) Self {
    _ = c.gladLoadGL(@ptrCast(c.GLADloadfunc, @alignCast(@alignOf(c.GLADloadfunc), p)));

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
    c.glEnableVertexAttribArray(0);
    c.glVertexAttribPointer(0, 2, c.GL_FLOAT, c.GL_FALSE, @sizeOf(Vertex), null);
    c.glEnableVertexAttribArray(1);
    c.glVertexAttribPointer(1, 2, c.GL_FLOAT, c.GL_FALSE, @sizeOf(Vertex), @intToPtr(*const anyopaque, @offsetOf(Vertex, "u")));
    c.glEnableVertexAttribArray(2);
    c.glVertexAttribPointer(2, 4, c.GL_UNSIGNED_BYTE, c.GL_TRUE, @sizeOf(Vertex), @intToPtr(*const anyopaque, @offsetOf(Vertex, "r")));
    ibo.bind();

    self.vao.unbind();
    vbo.unbind();
    ibo.unbind();

    return self;
}

pub fn begin(self: *Self, width: i32, height: i32, bg: []const f32) void {
    self.width = width;
    self.height = height;
    c.glViewport(0, 0, width, height);
    c.glScissor(0, 0, width, height);
    c.glClearColor(
        bg[0] / 255.0,
        bg[1] / 255.0,
        bg[2] / 255.0,
        1,
    );
    c.glClear(c.GL_COLOR_BUFFER_BIT);

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

        c.glDisable(c.GL_CULL_FACE);
        c.glDisable(c.GL_DEPTH_TEST);
        c.glEnable(c.GL_SCISSOR_TEST);
        c.glEnable(c.GL_TEXTURE_2D);

        // alpha blend
        c.glEnable(c.GL_BLEND);
        c.glBlendFunc(c.GL_SRC_ALPHA, c.GL_ONE_MINUS_SRC_ALPHA);

        // draw
        self.program.bind();
        self.program.set_uniform_matrix("M", &self.matrix[0], true);
        c.glActiveTexture(c.GL_TEXTURE0);
        self.atlas_texture.bind();
        self.vao.draw();
        self.atlas_texture.unbind();
        self.program.unbind();
    }
}

pub fn draw_rect(self: *Self, rect: c.UIRect, color: c.UIColor32) void {
    self.push_quad(rect, atlas.atlas[@enumToInt(atlas.ATLAS_GLYPH.ATLAS_WHITE)], color);
}

fn push_triangle(self: *Self, idx0: u32, idx1: u32, idx2: u32) void {
    self.indices[self.index_count] = idx0;
    self.index_count += 1;
    self.indices[self.index_count] = idx1;
    self.index_count += 1;
    self.indices[self.index_count] = idx2;
    self.index_count += 1;
}

fn push_quad(self: *Self, quad: c.UIRect, glyph: atlas.Rect, color: c.UIColor32) void {
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
