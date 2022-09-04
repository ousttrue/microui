const Self = @This();

x: i32 = 0,
y: i32 = 0,

pub fn append(self: *Self, rhs: Self) void {
    self.x += rhs.x;
    self.y += rhs.y;
}
