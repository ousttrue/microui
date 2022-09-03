const std = @import("std");
const c = @import("c");
pub const text_width_callback = fn (_: ?*anyopaque, text: [*c]const u8, _len: c_int) callconv(.C) c_int;
pub const text_height_callback = fn (_: ?*anyopaque) callconv(.C) c_int;
const ROOTLIST_SIZE = 32;
const COMMANDLIST_SIZE = 256 * 1024;
const Self = @This();

text_width: ?*const text_width_callback,
text_height: ?*const text_height_callback,
command_groups: [ROOTLIST_SIZE]c.struct_UICommandRange = undefined,
command_buffer: [COMMANDLIST_SIZE]u8 = undefined,

pub fn init(
    text_width: ?*const text_width_callback,
    text_height: ?*const text_height_callback,
) Self {
    return Self{
        .text_width = text_width,
        .text_height = text_height,
    };
}

pub fn deinit(self: Self) void {
    _ = self;
}

pub fn begin(self: Self) void {
    _ = self;
}

pub fn end(self: *Self, frame: *c.UIRenderFrame) !void {
    var fixed = std.heap.FixedBufferAllocator.init(&self.command_buffer);
    var buffer = std.ArrayList(u8).init(fixed.allocator());
    const w = buffer.writer();

    try w.writeIntNative(u32, c.UI_COMMAND_RECT);
    // c.UIRectCommand x, y, w, h, rgba 20bit
    try w.writeIntNative(i32, 10);
    try w.writeIntNative(i32, 10);
    try w.writeIntNative(i32, 10);
    try w.writeIntNative(i32, 10);
    try w.writeIntNative(u8, 255);
    try w.writeIntNative(u8, 255);
    try w.writeIntNative(u8, 255);
    try w.writeIntNative(u8, 255);
    self.command_groups[0].head = 0;
    self.command_groups[0].tail = @intCast(u32, buffer.items.len);

    frame.command_groups = &self.command_groups[0];
    frame.command_group_count = 1;
    frame.command_buffer = &self.command_buffer[0];
}
