const std = @import("std");
const c = @import("c");
const Vec2 = @import("./Vec2.zig");
const Rect = @import("./Rect.zig");
const Hash = @import("./Hash.zig");
const Container = @import("./Container.zig");

pub const MOUSE_BUTTON = enum(u32) {
    NONE = 0,
    LEFT = (1 << 0),
    RIGHT = (1 << 1),
    MIDDLE = (1 << 2),

    pub fn remove(self: @This(), rhs: @This()) @This() {
        return @intToEnum(@This(), @enumToInt(self) & ~@enumToInt(rhs));
    }
};

pub const KEY = enum(u32) {
    NONE = 0,
    SHIFT = (1 << 0),
    CTRL = (1 << 1),
    ALT = (1 << 2),
    BACKSPACE = (1 << 3),
    RETURN = (1 << 4),

    pub fn has(self: @This(), rhs: @This()) bool {
        return (@enumToInt(self) & @enumToInt(rhs)) != 0;
    }

    pub fn add(self: @This(), rhs: @This()) @This() {
        return @intToEnum(@This(), @enumToInt(self) | @enumToInt(rhs));
    }

    pub fn remove(self: @This(), rhs: @This()) @This() {
        return @intToEnum(@This(), @enumToInt(self) & ~@enumToInt(rhs));
    }
};

pub const OPT = enum(u32) {
    NONE = 0,
    ALIGNCENTER = (1 << 0),
    ALIGNRIGHT = (1 << 1),
    NOINTERACT = (1 << 2),
    NOFRAME = (1 << 3),
    NORESIZE = (1 << 4),
    NOSCROLL = (1 << 5),
    NOCLOSE = (1 << 6),
    NOTITLE = (1 << 7),
    HOLDFOCUS = (1 << 8),
    AUTOSIZE = (1 << 9),
    POPUP = (1 << 10),
    CLOSED = (1 << 11),
    EXPANDED = (1 << 12),

    pub fn has(self: OPT, opt: OPT) bool {
        return (@enumToInt(self) & @enumToInt(opt)) != 0;
    }

    pub fn add(self: @This(), rhs: @This()) @This() {
        return @intToEnum(@This(), @enumToInt(self) | @enumToInt(rhs));
    }
};

pub const RES = enum(u32) {
    NONE = 0,
    ACTIVE = (1 << 0),
    SUBMIT = (1 << 1),
    CHANGE = (1 << 2),

    pub fn has(self: @This(), rhs: @This()) bool {
        return (@enumToInt(self) & @enumToInt(rhs)) != 0;
    }

    pub fn add(self: @This(), rhs: @This()) @This() {
        return @intToEnum(@This(), @enumToInt(self) | @enumToInt(rhs));
    }
};

pub const FOCUS_STATE = enum {
    NONE,
    HOVER,
    FOCUS,
};

const Self = @This();

mouse_pos: Vec2 = .{},
last_mouse_pos: Vec2 = .{},
mouse_delta: Vec2 = .{},
scroll_delta: Vec2 = .{},
mouse_down: MOUSE_BUTTON = .NONE,
mouse_pressed: MOUSE_BUTTON = .NONE,
key_down: KEY = .NONE,
key_pressed: KEY = .NONE,
input_text: [32]u8 = .{0} ** 32,
input_text_pos: usize = 0,

focus: ?Hash.Id = null,
keep_focus: bool = false,
hover: ?Hash.Id = null,

scroll_target: ?*Container = null,

pub fn begin(self: *Self) void {
    self.mouse_delta.x = self.mouse_pos.x - self.last_mouse_pos.x;
    self.mouse_delta.y = self.mouse_pos.y - self.last_mouse_pos.y;
    self.scroll_target = null;
}

pub fn end(self: *Self) MOUSE_BUTTON {
    // unset focus if focus id was not touched this frame
    if (self.keep_focus) {
        self.keep_focus = false;
    } else {
        self.focus = null;
    }

    // handle scroll input
    const mouse_pressed = self.mouse_pressed;
    if (self.scroll_target) |scroll_target| {
        scroll_target.scroll.append(self.scroll_delta);
    }

    // reset input state
    self.key_pressed = .NONE;
    self.input_text[0] = 0;
    self.mouse_pressed = .NONE;
    self.scroll_delta = .{};
    self.last_mouse_pos = self.mouse_pos;

    return mouse_pressed;
}

pub fn set_scroll_target(self: *Self, cnt: *Container) void {
    self.scroll_target = cnt;
}

pub fn set_mousemove(self: *Self, x: i32, y: i32) void {
    self.mouse_pos = .{ .x = x, .y = y };
}

pub fn set_mousedown(self: *Self, btn: MOUSE_BUTTON) void {
    self.mouse_down = @intToEnum(MOUSE_BUTTON, @enumToInt(self.mouse_down) | @enumToInt(btn));
    self.mouse_pressed = @intToEnum(MOUSE_BUTTON, @enumToInt(self.mouse_pressed) | @enumToInt(btn));
}

pub fn set_mouseup(self: *Self, btn: MOUSE_BUTTON) void {
    self.mouse_down = self.mouse_down.remove(btn);
}

pub fn set_scroll(self: *Self, x: i32, y: i32) void {
    self.scroll_delta.x += x;
    self.scroll_delta.y += y;
}

pub fn set_keydown(self: *Self, key: KEY) void {
    self.key_pressed = self.key_pressed.add(key);
    self.key_down = self.key_down.add(key);
}

pub fn set_keyup(self: *Self, key: KEY) void {
    self.key_down = self.key_down.remove(key);
}

pub fn push_text(self: *Self, text: []const u8) void {
    @memcpy(
        @ptrCast([*]u8, &self.input_text[self.input_text_pos]),
        @ptrCast([*]const u8, &text[0]),
        text.len,
    );
    self.input_text_pos += text.len;
}

pub fn set_focus(self: *Self, id: Hash.Id) void {
    self.focus = id;
    self.keep_focus = true;
}

pub fn set_keep_focus(self: *Self, id: Hash.Id) void {
    if (self.focus == id) {
        self.keep_focus = true;
    }
}

pub fn has_focus(self: Self, id: Hash.Id) bool {
    return self.focus == id;
}

pub fn set_hover(self: *Self, id: Hash.Id) void {
    self.hover = id;
}

pub fn has_hover(self: Self, id: Hash.Id) bool {
    return self.hover == id;
}

pub fn get_focus_state(self: Self, id: Hash.Id) FOCUS_STATE {
    if (self.has_focus(id)) {
        return .FOCUS;
    } else if (self.hover == id) {
        return .HOVER;
    } else {
        return .NONE;
    }
}

pub fn update_focus_hover(self: *Self, id: Hash.Id, opt: OPT, mouseover: bool) void {
    self.set_keep_focus(id);
    if (opt.has(.NOINTERACT)) {
        return;
    }

    if (mouseover and self.mouse_down == .NONE) {
        self.set_hover(id);
    }

    if (self.has_focus(id)) {
        if (self.mouse_pressed != .NONE and !mouseover) {
            self.set_focus(0);
        }
        if (self.mouse_down == .NONE and !opt.has(.HOLDFOCUS)) {
            self.set_focus(0);
        }
    }

    if (self.has_hover(id)) {
        if (self.mouse_pressed != .NONE) {
            self.set_focus(id);
        } else if (!mouseover) {
            self.set_hover(0);
        }
    }
}

fn consume_text(self: *Self, buf: []u8) usize {
    const n = std.math.min(buf.len - 1, self.input_text_pos);
    if (n > 0) {
        @memcpy(@ptrCast([*]u8, &buf[0]), @ptrCast([*]const u8, &self.input_text[0]), n);
    }
    if (n < buf.len) {
        buf[n] = 0;
    }
    self.input_text_pos = 0;
    return n;
}

pub const HandleResult = struct {
    res: RES = .NONE,
    size: usize = 0,
};

pub fn handle_text(self: *Self, id: Hash.Id, buf: []u8) HandleResult {
    var result = HandleResult{};
    if (self.has_focus(id)) {
        result.size = @intCast(usize, c.strlen(&buf[0]));

        // handle text input
        const n = self.consume_text(buf[result.size..]);
        if (n > 0) {
            result.res = result.res.add(.CHANGE);
            result.size += n;
        }

        // handle backspace
        if (self.key_pressed.has(.BACKSPACE) and result.size > 0) {
            // skip utf-8 continuation bytes
            // while ((buf[--len] & 0xc0) == 0x80 && len > 0)
            //   ;
            // buf[len] = '\0';
            // res = static_cast<MU_RES>(res | MU_RES_CHANGE);
        }

        // handle return
        if (self.key_pressed.has(.RETURN)) {
            self.set_focus(0);
            result.res = result.res.add(.SUBMIT);
        }
    }

    return result;
}
