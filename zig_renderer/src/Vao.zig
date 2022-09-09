const c = @import("c");
const Vbo = @import("./Vbo.zig");
const Ibo = @import("./Ibo.zig");
const Self = @This();

id: u32,
vbo: Vbo,
ibo: Ibo,

pub fn init(vbo: Vbo, ibo: Ibo) Self {
    var id: u32 = undefined;
    c.glGenVertexArrays(1, &id);
    return Self{
        .id = id,
        .vbo = vbo,
        .ibo = ibo,
    };
}

pub fn deinit(self: Self) void {
    c.glDeleteVertexArrays(1, &self.id);
}

pub fn bind(self: Self) void {
    c.glBindVertexArray(self.id);
}
pub fn unbind(_: Self) void {
    c.glBindVertexArray(0);
}

pub fn draw(self: Self) void {
    self.bind();
    self.ibo.draw();
    self.unbind();
}
