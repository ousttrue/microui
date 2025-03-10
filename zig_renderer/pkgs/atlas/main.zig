const std = @import("std");
pub const data = @embedFile("./atlas.bin");
pub const width = 128;
pub const height = 128;

pub const ATLAS_GLYPH = enum(u32) {
    MU_ICON_CLOSE = 1,
    MU_ICON_CHECK,
    MU_ICON_COLLAPSED,
    MU_ICON_EXPANDED,
    ATLAS_WHITE = 5,
    ATLAS_FONT,
};

pub const Rect = std.meta.Tuple(&.{ i32, i32, i32, i32 });

pub fn get_text_width(text: []const u8) u32 {
    var res: i32 = 0;
    for (text) |ch| {
        if ((ch & 0xc0) == 0x80) {
            continue;
        }
        const chr = std.math.min(ch, 127);
        res += atlas[chr][2];
    }
    return @intCast(u32, res);
}

pub fn zigmui_width(_: ?*anyopaque, text: []const u8) u32 {
    if (text.len == 0) {
        return 0;
    }
    return get_text_width(text);
}

pub fn get_text_height() u32 {
    return 18;
}


pub fn zigmui_height(_: ?*anyopaque) u32 {
    return get_text_height();
}

pub const atlas = [_]Rect{
    .{ 0, 0, 0, 0 },
    .{ 88, 68, 16, 16 },
    .{ 0, 0, 18, 18 },
    .{ 118, 68, 7, 5 },
    .{ 113, 68, 5, 7 },
    .{ 125, 68, 3, 3 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 }, // 10
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 }, // 20
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 },
    .{ 0, 0, 0, 0 }, // 30
    .{ 0, 0, 0, 0 },
    .{ 84, 68, 2, 17 }, // 32
    .{ 39, 68, 3, 17 },
    .{ 114, 51, 5, 17 },
    .{ 34, 17, 7, 17 },
    .{ 28, 34, 6, 17 },
    .{ 58, 0, 9, 17 },
    .{ 103, 0, 8, 17 },
    .{ 86, 68, 2, 17 },
    .{ 42, 68, 3, 17 }, // 40
    .{ 45, 68, 3, 17 },
    .{ 34, 34, 6, 17 },
    .{ 40, 34, 6, 17 },
    .{ 48, 68, 3, 17 },
    .{ 51, 68, 3, 17 },
    .{ 54, 68, 3, 17 },
    .{ 124, 34, 4, 17 },
    .{ 46, 34, 6, 17 },
    .{ 52, 34, 6, 17 },
    .{ 58, 34, 6, 17 }, // 50
    .{ 64, 34, 6, 17 },
    .{ 70, 34, 6, 17 },
    .{ 76, 34, 6, 17 },
    .{ 82, 34, 6, 17 },
    .{ 88, 34, 6, 17 },
    .{ 94, 34, 6, 17 },
    .{ 100, 34, 6, 17 },
    .{ 57, 68, 3, 17 },
    .{ 60, 68, 3, 17 },
    .{ 106, 34, 6, 17 }, // 60
    .{ 112, 34, 6, 17 },
    .{ 118, 34, 6, 17 },
    .{ 119, 51, 5, 17 },
    .{ 18, 0, 10, 17 },
    .{ 41, 17, 7, 17 },
    .{ 48, 17, 7, 17 },
    .{ 55, 17, 7, 17 },
    .{ 111, 0, 8, 17 },
    .{ 0, 35, 6, 17 },
    .{ 6, 35, 6, 17 }, // 70
    .{ 119, 0, 8, 17 },
    .{ 18, 17, 8, 17 },
    .{ 63, 68, 3, 17 },
    .{ 66, 68, 3, 17 },
    .{ 62, 17, 7, 17 },
    .{ 12, 51, 6, 17 },
    .{ 28, 0, 10, 17 },
    .{ 67, 0, 9, 17 },
    .{ 76, 0, 9, 17 },
    .{ 69, 17, 7, 17 }, // 80
    .{ 85, 0, 9, 17 },
    .{ 76, 17, 7, 17 },
    .{ 18, 51, 6, 17 },
    .{ 24, 51, 6, 17 },
    .{ 26, 17, 8, 17 },
    .{ 83, 17, 7, 17 },
    .{ 38, 0, 10, 17 },
    .{ 90, 17, 7, 17 },
    .{ 30, 51, 6, 17 },
    .{ 36, 51, 6, 17 }, // 90
    .{ 69, 68, 3, 17 },
    .{ 124, 51, 4, 17 },
    .{ 72, 68, 3, 17 },
    .{ 42, 51, 6, 17 },
    .{ 15, 68, 4, 17 },
    .{ 48, 51, 6, 17 },
    .{ 54, 51, 6, 17 },
    .{ 97, 17, 7, 17 },
    .{ 0, 52, 5, 17 },
    .{ 104, 17, 7, 17 }, // 100
    .{ 60, 51, 6, 17 },
    .{ 19, 68, 4, 17 },
    .{ 66, 51, 6, 17 },
    .{ 111, 17, 7, 17 },
    .{ 75, 68, 3, 17 },
    .{ 78, 68, 3, 17 },
    .{ 72, 51, 6, 17 },
    .{ 81, 68, 3, 17 },
    .{ 48, 0, 10, 17 },
    .{ 118, 17, 7, 17 }, // 110
    .{ 0, 18, 7, 17 },
    .{ 7, 18, 7, 17 },
    .{ 14, 34, 7, 17 },
    .{ 23, 68, 4, 17 },
    .{ 5, 52, 5, 17 },
    .{ 27, 68, 4, 17 },
    .{ 21, 34, 7, 17 },
    .{ 78, 51, 6, 17 },
    .{ 94, 0, 9, 17 },
    .{ 84, 51, 6, 17 }, // 120
    .{ 90, 51, 6, 17 },
    .{ 10, 68, 5, 17 },
    .{ 31, 68, 4, 17 },
    .{ 96, 51, 6, 17 },
    .{ 35, 68, 4, 17 },
    .{ 102, 51, 6, 17 },
    .{ 108, 51, 6, 17 }, // 127
};
