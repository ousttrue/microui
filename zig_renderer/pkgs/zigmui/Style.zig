const std = @import("std");
const Vec2 = @import("./Vec2.zig");
const Rect = @import("./Rect.zig");
const Color32 = @import("./Color32.zig");
const Input = @import("./Input.zig");
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

pub const text_width_callback = fn (_: ?*anyopaque, text: []const u8) u32;
pub const text_height_callback = fn (_: ?*anyopaque) u32;

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
        return @intCast(c_int, callback(self.font, str));
    } else {
        unreachable;
    }
}

pub fn text_height(self: Self) c_int {
    if (self.text_height_callback) |callback| {
        return @intCast(c_int, callback(self.font));
    } else {
        unreachable;
    }
}

pub fn text_position(
    self: Self,
    rect: Rect,
    str: []const u8,
    opt: Input.OPT,
    edit_size: ?*Vec2,
) Vec2 {
    const tw = self.text_width(str);
    const th = self.text_height();
    if (edit_size) |es| {
        es.x = tw;
        es.y = th;
    }
    var pos: Vec2 = undefined;
    if (opt.has(.ALIGNCENTER)) {
        pos.x = rect.x + @divTrunc(rect.w - tw, @as(i32, 2));
    } else if (opt.has(.ALIGNRIGHT)) {
        pos.x = rect.x + rect.w - tw - self.padding;
    } else {
        if (edit_size != null) {
            const ofx = rect.w - self.padding - tw - 1;
            pos.x = rect.x + std.math.min(ofx, self.padding);
        } else {
            pos.x = rect.x + self.padding;
        }
    }
    pos.y = rect.y + @divTrunc(rect.h - th, @as(i32, 2));
    return pos;
}
