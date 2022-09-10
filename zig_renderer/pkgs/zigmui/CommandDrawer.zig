const c = @import("c");
const Stack = @import("./stack.zig").Stack;
const Rect = @import("./Rect.zig");
const Vec2 = @import("./Vec2.zig");
const Color32 = @import("./Color32.zig");
const ClipStack = @import("./ClipStack.zig");
const Style = @import("./Style.zig");
const Input = @import("./Input.zig");
const COMMANDLIST_SIZE = (256 * 1024);

pub const COMMAND = enum(u32) {
    CLIP,
    RECT,
    TEXT,
    ICON,
};

pub const ICON = enum(i32) {
    CLOSE = 1,
    CHECK,
    COLLAPSED,
    EXPANDED,
    MAX,
};

const Self = @This();

buffer: [COMMANDLIST_SIZE]u8 = undefined,
pos: usize = 0,
clip_stack: ClipStack = .{},
style: Style = .{},

pub fn begin(self: *Self) void {
    self.pos = 0;
}

pub fn end(self: *Self, command: *c.UIRenderFrame) void {
    self.clip_stack.end();
    command.command_buffer = @ptrCast([*c]u8, &self.buffer[0]);
}

fn write_bytes(self: *Self, value: anytype) void {
    const T = @TypeOf(value);
    const size = @sizeOf(T);
    @memcpy(@ptrCast([*]u8, &self.buffer[self.pos]), @ptrCast([*]const u8, &value), size);
    self.pos += size;
}

fn write(self: *Self, data: []const u8) void {
    if (data.len == 0) {
        return;
    }
    @memcpy(@ptrCast([*]u8, &self.buffer[self.pos]), @ptrCast([*]const u8, &data[0]), data.len);
    self.pos += data.len;
}

pub fn write_clip(self: *Self, rect: Rect) void {
    self.write_bytes(c.UI_COMMAND_CLIP);
    const value = c.UIClipCommand{
        .rect = .{
            .x = rect.x,
            .y = rect.y,
            .w = rect.w,
            .h = rect.h,
        },
    };
    self.write_bytes(value);
}

pub fn write_text(self: *Self, str: []const u8, pos: Vec2, color: Color32, font: ?*const anyopaque) void {
    // TODO: text の埋め方を変える
    // 取り出すときに
    // @setRuntimeSafety(false);
    // が必要になっている。
    self.write_bytes(c.UI_COMMAND_TEXT);
    const value = c.UITextCommand{
        .pos = .{
            .x = pos.x,
            .y = pos.y,
        },
        .color = .{
            .r = color.r,
            .g = color.g,
            .b = color.b,
            .a = color.a,
        },
        .length = @intCast(u32, str.len),
        .font = font,
    };
    self.write_bytes(value);
    self.write(str);
}

pub fn write_icon(self: *Self, id: ICON, rect: Rect, color: Color32) void {
    self.write_bytes(c.UI_COMMAND_ICON);
    const value = c.UIIconCommand{
        .rect = .{
            .x = rect.x,
            .y = rect.y,
            .w = rect.w,
            .h = rect.h,
        },
        .id = @enumToInt(id),
        .color = .{
            .r = color.r,
            .g = color.g,
            .b = color.b,
            .a = color.a,
        },
    };
    self.write_bytes(value);
}

pub fn write_rect(self: *Self, rect: Rect, color: Color32) void {
    self.write_bytes(c.UI_COMMAND_RECT);
    const value = c.UIRectCommand{
        .rect = .{
            .x = rect.x,
            .y = rect.y,
            .w = rect.w,
            .h = rect.h,
        },
        .color = .{
            .r = color.r,
            .g = color.g,
            .b = color.b,
            .a = color.a,
        },
    };
    self.write_bytes(value);
}

pub fn draw_rect(self: *Self, rect: Rect, color: Color32) void {
    self.write_rect(self.clip_stack.intersect(rect), color);
}

pub fn draw_box(self: *Self, rect: Rect, color: Color32) void {
    self.draw_rect(Rect{ .x = rect.x + 1, .y = rect.y, .w = rect.w - 2, .h = 1 }, color);
    self.draw_rect(Rect{ .x = rect.x + 1, .y = rect.y + rect.h - 1, .w = rect.w - 2, .h = 1 }, color);
    self.draw_rect(Rect{ .x = rect.x, .y = rect.y, .w = 1, .h = rect.h }, color);
    self.draw_rect(Rect{ .x = rect.x + rect.w - 1, .y = rect.y, .w = 1, .h = rect.h }, color);
}

pub fn draw_text(self: *Self, str: []const u8, pos: Vec2, colorid: Style.STYLE) void {
    const color = self.style.colors[@enumToInt(colorid)];
    const rect =
        Rect{ .x = pos.x, .y = pos.y, .w = self.style.text_width(str), .h = self.style.text_height() };
    const clipped = self.clip_stack.check_clip(rect);
    if (clipped == .ALL) {
        return;
    }
    if (clipped == .PART) {
        self.write_clip(self.clip_stack.back_const());
    }
    // add command
    self.write_text(str, pos, color, self.style.font);
    // reset clipping if it was set
    if (clipped != .NONE) {
        self.write_clip(Rect.UNCLIPPED_RECT);
    }
}

pub fn draw_icon(self: *Self, id: ICON, rect: Rect, colorid: Style.STYLE) void {
    // do clip command if the rect isn't fully contained within the cliprect
    const clipped = self.clip_stack.check_clip(rect);
    if (clipped == .ALL) {
        return;
    }
    if (clipped == .PART) {
        self.write_clip(self.clip_stack.back_const());
    }
    // do icon command
    self.write_icon(id, rect, self.style.colors[@enumToInt(colorid)]);
    // reset clipping if it was set
    if (clipped != .NONE) {
        self.write_clip(Rect.UNCLIPPED_RECT);
    }
}

pub fn draw_frame(self: *Self, rect: Rect, colorid: Style.STYLE) void {
    self.draw_rect(rect, self.style.colors[@enumToInt(colorid)]);
    if (colorid == .SCROLLBASE or colorid == .SCROLLTHUMB or
        colorid == .TITLEBG)
    {
        return;
    }
    // draw border
    if (self.style.colors[@enumToInt(Style.STYLE.BORDER)].a > 0) {
        self.draw_box(rect.expand(1), self.style.colors[@enumToInt(Style.STYLE.BORDER)]);
    }
}

pub fn draw_control_frame(self: *Self, rect: Rect, colorid: Style.STYLE, opt: Input.OPT, focus_state: Input.FOCUS_STATE) void {
    if (opt.has(.NOFRAME)) {
        return;
    }
    self.draw_frame(rect, @intToEnum(Style.STYLE, @enumToInt(colorid) + @enumToInt(focus_state)));
}

pub fn draw_control_text(
    self: *Self,
    str: []const u8,
    rect: Rect,
    colorid: Style.STYLE,
    opt: Input.OPT,
    edit: bool,
) void {
    self.clip_stack.push(rect);
    if (edit) {
        var size: Vec2 = undefined;
        const pos = self.style.text_position(rect, str, .NONE, &size);
        self.draw_text(str, pos, .TEXT);
        // caret
        self.draw_rect(
            Rect{ .x = pos.x + size.x, .y = pos.y, .w = 1, .h = size.y },
            self.style.colors[@enumToInt(Style.STYLE.TEXT)],
        );
    } else {
        const pos = self.style.text_position(rect, str, opt, null);
        self.draw_text(str, pos, colorid);
    }
    self.clip_stack.pop();
}
