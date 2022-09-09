const c = @import("c");
const Texture = @import("./Texture.zig");
const Self = @This();

atlas_texture: Texture = undefined,

pub fn init() Self {
    return Self{};
}

pub fn loadfunc(self: Self, p: *const anyopaque) void {
    _ = self;
    _ = c.gladLoadGL(@ptrCast(c.GLADloadfunc, @alignCast(@alignOf(c.GLADloadfunc), p)));

    //   shader = Program::create(VS, FS);
    //   vao = VAO::create();
    //   vao->vbo = VBO::create(MAX_QUADS * 6 * sizeof(Vertex));
    //   vao->ibo = IBO::create(MAX_QUADS * 4 * sizeof(uint32_t));

    //   // https://www.khronos.org/opengl/wiki/Vertex_Specification
    //   vao->bind();
    //   vao->vbo->bind();
    //   glEnableVertexAttribArray(0);
    //   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    //   glEnableVertexAttribArray(1);
    //   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    //                         (void *)offsetof(Vertex, u));
    //   glEnableVertexAttribArray(2);
    //   glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
    //                         (void *)offsetof(Vertex, color));
    //   vao->ibo->bind();
    //   vao->unbind();
    //   vao->vbo->unbind();
    //   vao->ibo->unbind();
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
