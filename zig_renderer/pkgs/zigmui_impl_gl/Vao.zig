const gl = @import("gl");
const Vbo = @import("./Vbo.zig");
const Ibo = @import("./Ibo.zig");
const Self = @This();

id: u32,
vbo: Vbo,
ibo: Ibo,

pub fn init(vbo: Vbo, ibo: Ibo) Self {
    var id: u32 = undefined;
    gl.genVertexArrays(1, &id);
    return Self{
        .id = id,
        .vbo = vbo,
        .ibo = ibo,
    };
}

pub fn deinit(self: Self) void {
    gl.deleteVertexArrays(1, &self.id);
}

pub fn bind(self: Self) void {
    gl.bindVertexArray(self.id);
}
pub fn unbind(_: Self) void {
    gl.bindVertexArray(0);
}

pub fn draw(self: Self) void {
    self.bind();
    self.ibo.draw();
    self.unbind();
}
