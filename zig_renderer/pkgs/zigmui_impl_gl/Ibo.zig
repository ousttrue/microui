const gl = @import("gl");
const Self = @This();

id: u32,
count: u32 = 0,

pub fn init(size: u32) Self {
    var id: u32 = undefined;
    gl.genBuffers(1, &id);
    var self = Self{
        .id = id,
    };
    self.bind();
    gl.bufferData(gl.GL_ELEMENT_ARRAY_BUFFER, size, null, gl.GL_DYNAMIC_DRAW);
    self.unbind();
    return self;
}

pub fn bind(self: Self) void {
    gl.bindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.id);
}

pub fn unbind(_: Self) void {
    gl.bindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, 0);
}

pub fn update(self: *Self, p: *const anyopaque, size: u32, count: u32) void {
    self.bind();
    gl.bufferSubData(gl.GL_ELEMENT_ARRAY_BUFFER, 0, size, p);
    self.count = count;
    self.unbind();
}

pub fn draw(self: Self) void {
    gl.drawElements(gl.GL_TRIANGLES, @intCast(c_int, self.count), gl.GL_UNSIGNED_INT, 0);
}
