const Vec2 = @import("./Vec2.zig");
const Rect = @import("./Rect.zig");
const Hash = @import("./Hash.zig");
const Container = @import("./Container.zig");

pub const MOUSE_BUTTON = enum(u32) {
    NONE = 0,
    LEFT = (1 << 0),
    RIGHT = (1 << 1),
    MIDDLE = (1 << 2),

    pub fn remove(self: *@This(), rhs: @This()) void {
        self.* = @intToEnum(@This(), @enumToInt(self.*) & ~@enumToInt(rhs));
    }
};

pub const KEY = enum(u32) {
    NONE = 0,
    SHIFT = (1 << 0),
    CTRL = (1 << 1),
    ALT = (1 << 2),
    BACKSPACE = (1 << 3),
    RETURN = (1 << 4),
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

    pub fn contains(self: OPT, opt: OPT) bool {
        return (@enumToInt(self) & @enumToInt(opt)) != 0;
    }
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

pub fn input_mousemove(self: *Self, x: i32, y: i32) void {
    self.mouse_pos = .{ .x = x, .y = y };
}

pub fn input_mousedown(self: *Self, btn: MOUSE_BUTTON) void {
    self.mouse_down = @intToEnum(MOUSE_BUTTON, @enumToInt(self.mouse_down) | @enumToInt(btn));
    self.mouse_pressed = @intToEnum(MOUSE_BUTTON, @enumToInt(self.mouse_pressed) | @enumToInt(btn));
}

pub fn input_mouseup(self: *Self, btn: MOUSE_BUTTON) void {
    self.mouse_down.remove(btn);
}

pub fn input_scroll(self: *Self, x: i32, y: i32) void {
    self.scroll_delta.x += x;
    self.scroll_delta.y += y;
}

//   void keydown(MU_KEY key) {
//     self._key_pressed = self._key_pressed | key;
//     self._key_down = self._key_down | key;
//   }

//   void keyup(MU_KEY key) {
//     self._key_down = self._key_down & static_cast<MU_KEY>(~key);
//   }

//   void text(const char *text) {
//     int len = strlen(self._input_text);
//     int size = strlen(text) + 1;
//     assert(len + size <= (int)sizeof(self._input_text));
//     memcpy(self._input_text + len, text, size);
//   }

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

//   FOCUS_STATE get_focus_state(mu_Id id) const {
//     return has_focus(id)  ? FOCUS_STATE_FOCUS
//            : _hover == id ? FOCUS_STATE_HOVER
//                           : FOCUS_STATE_NONE;
//   }

pub fn update_focus_hover(self: *Self, id: Hash.Id, opt: OPT, mouseover: bool) void {
    self.set_keep_focus(id);
    if (opt.contains(.NOINTERACT)) {
        return;
    }

    if (mouseover and self.mouse_down == .NONE) {
        self.set_hover(id);
    }

    if (self.has_focus(id)) {
        if (self.mouse_pressed != .NONE and !mouseover) {
            self.set_focus(0);
        }
        if (self.mouse_down == .NONE and !opt.contains(.HOLDFOCUS)) {
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
