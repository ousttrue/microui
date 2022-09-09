const c = @import("c");
const Self = @This();

id: u32,
count: u32 = 0,

pub fn init(size: u32) Self {
    var id: u32 = undefined;
    c.glGenBuffers(1, &id);
    var self = Self{
        .id = id,
    };
    self.bind();
    c.glBufferData(c.GL_ELEMENT_ARRAY_BUFFER, size, null, c.GL_DYNAMIC_DRAW);
    self.unbind();
    return self;
}

pub fn bind(self: Self) void {
    c.glBindBuffer(c.GL_ELEMENT_ARRAY_BUFFER, self.id);
}

pub fn unbind(_: Self) void {
    c.glBindBuffer(c.GL_ELEMENT_ARRAY_BUFFER, 0);
}

pub fn update(self: Self, values: anytype) void {
    const A = @TypeOf(values[0]);
    switch (@typeInfo(A)) {
        .Pointer => |pointer| {
            self.bind();
            c.glBufferSubData(c.GL_ELEMENT_ARRAY_BUFFER, 0, @sizeOf(pointer.child) * pointer.size, &values[0]);
            self.count = values.len;
            unbind();
        },
        else => {
            unreachable;
        },
    }
}

pub fn draw(self: Self) void {
    c.glDrawElements(c.GL_TRIANGLES, @intCast(c_int, self.count), c.GL_UNSIGNED_INT, null);
}
