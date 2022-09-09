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
    c.glBufferData(c.GL_ARRAY_BUFFER, size, null, c.GL_DYNAMIC_DRAW);
    self.unbind();
    return self;
}

pub fn bind(self: Self) void {
    c.glBindBuffer(c.GL_ARRAY_BUFFER, self.id);
}

pub fn unbind(_: Self) void {
    c.glBindBuffer(c.GL_ARRAY_BUFFER, 0);
}

pub fn update(self: *Self, p: *const anyopaque, size: u32, count: u32) void {
    self.bind();
    c.glBufferSubData(c.GL_ARRAY_BUFFER, 0, size, p);
    self.count = count;
    self.unbind();
}

