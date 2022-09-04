const Vec2 = @import("./Vec2.zig");
const Hash = @import("./Hash.zig");
const Container = @import("./Container.zig");

pub const MOUSE_BUTTON = enum(u32) {
    NONE = 0,
    LEFT = (1 << 0),
    RIGHT = (1 << 1),
    MIDDLE = (1 << 2),
};

pub const KEY = enum(u32) {
    NONE = 0,
    SHIFT = (1 << 0),
    CTRL = (1 << 1),
    ALT = (1 << 2),
    BACKSPACE = (1 << 3),
    RETURN = (1 << 4),
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

//   void mousemove(int x, int y) { this->_mouse_pos = UIVec2(x, y); }

//   void mousedown(MU_MOUSE_BUTTON btn) {
//     this->_mouse_down = this->_mouse_down | btn;
//     this->_mouse_pressed = this->_mouse_pressed | btn;
//   }

//   void mouseup(MU_MOUSE_BUTTON btn) {
//     this->_mouse_down = this->_mouse_down & static_cast<MU_MOUSE_BUTTON>(~btn);
//   }

//   void scroll(int x, int y) {
//     this->_scroll_delta.x += x;
//     this->_scroll_delta.y += y;
//   }

//   void keydown(MU_KEY key) {
//     this->_key_pressed = this->_key_pressed | key;
//     this->_key_down = this->_key_down | key;
//   }

//   void keyup(MU_KEY key) {
//     this->_key_down = this->_key_down & static_cast<MU_KEY>(~key);
//   }

//   void text(const char *text) {
//     int len = strlen(this->_input_text);
//     int size = strlen(text) + 1;
//     assert(len + size <= (int)sizeof(this->_input_text));
//     memcpy(this->_input_text + len, text, size);
//   }
