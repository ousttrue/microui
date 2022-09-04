const Vec2 = @import("./Vec2.zig");
const Color32 = @import("./Color32.zig");
pub const STYLE = enum(u32) {
    TEXT,
    BORDER,
    WINDOWBG,
    TITLEBG,
    TITLETEXT,
    PANELBG,
    BUTTON,
    BUTTONHOVER,
    BUTTONFOCUS,
    BASE,
    BASEHOVER,
    BASEFOCUS,
    SCROLLBASE,
    SCROLLTHUMB,
    MAX,
};

pub const text_width_callback = fn (_: ?*anyopaque, text: [*c]const u8, _len: c_int) callconv(.C) c_int;
pub const text_height_callback = fn (_: ?*anyopaque) callconv(.C) c_int;

const Self = @This();
text_width_callback: ?*const text_width_callback = null,
text_height_callback: ?*const text_height_callback = null,

font: ?*anyopaque = null,
size: Vec2 = .{ .x = 68, .y = 10 },
padding: c_int = 5,
spacing: c_int = 4,
indent: c_int = 24,
title_height: c_int = 24,
scrollbar_size: c_int = 12,
thumb_size: c_int = 8,
colors: [@enumToInt(STYLE.MAX)]Color32 = .{
    .{ .r = 230, .g = 230, .b = 230, .a = 255 }, // MU_COLOR_TEXT
    .{ .r = 25, .g = 25, .b = 25, .a = 255 }, // MU_COLOR_BORDER
    .{ .r = 50, .g = 50, .b = 50, .a = 255 }, // MU_COLOR_WINDOWBG
    .{ .r = 25, .g = 25, .b = 25, .a = 255 }, // MU_COLOR_TITLEBG
    .{ .r = 240, .g = 240, .b = 240, .a = 255 }, // MU_COLOR_TITLETEXT
    .{ .r = 0, .g = 0, .b = 0, .a = 0 }, // MU_COLOR_PANELBG
    .{ .r = 75, .g = 75, .b = 75, .a = 255 }, // MU_COLOR_BUTTON
    .{ .r = 95, .g = 95, .b = 95, .a = 255 }, // MU_COLOR_BUTTONHOVER
    .{ .r = 115, .g = 115, .b = 115, .a = 255 }, // MU_COLOR_BUTTONFOCUS
    .{ .r = 30, .g = 30, .b = 30, .a = 255 }, // MU_COLOR_BASE
    .{ .r = 35, .g = 35, .b = 35, .a = 255 }, // MU_COLOR_BASEHOVER
    .{ .r = 40, .g = 40, .b = 40, .a = 255 }, // MU_COLOR_BASEFOCUS
    .{ .r = 43, .g = 43, .b = 43, .a = 255 }, // MU_COLOR_SCROLLBASE
    .{ .r = 30, .g = 30, .b = 30, .a = 255 }, // MU_COLOR_SCROLLTHUMB
},

pub fn text_width(self: Self, str: []const u8) c_int {
    if (self.text_width_callback) |callback| {
        return callback.*(self.font, &str[0], @intCast(c_int, str.len));
    } else {
        unreachable;
    }
}

pub fn text_height(self: Self) c_int {
    if (self.text_height_callback) |callback| {
        return callback.*(self.font);
    } else {
        unreachable;
    }
}
