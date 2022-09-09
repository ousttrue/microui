const std = @import("std");
const c = @import("c");
const ui = @import("./ui.zig");
const zigmui = @import("zigmui");

fn text_width(_: ?*anyopaque, text: []const u8) u32 {
    if (text.len == 0) {
        return 0;
    }
    return @intCast(u32, c.MUI_RENDERER_get_text_width(&text[0], @intCast(c_int, text.len)));
}

fn text_height(_: ?*anyopaque) u32 {
    return @intCast(u32, c.MUI_RENDERER_get_text_height());
}

var mouse_x: c_int = 0;
var mouse_y: c_int = 0;
fn cursor_position_callback(window: ?*c.GLFWwindow, xpos: f64, ypos: f64) callconv(.C) void {
    const ctx = @ptrCast(*zigmui.Context, @alignCast(@alignOf(zigmui.Context), c.glfwGetWindowUserPointer(window)));
    mouse_x = @floatToInt(c_int, xpos);
    mouse_y = @floatToInt(c_int, ypos);
    ctx.input.set_mousemove(mouse_x, mouse_y);
}

fn mouse_button_callback(
    window: ?*c.GLFWwindow,
    button: c_int,
    action: c_int,
    _: c_int,
) callconv(.C) void {
    const ctx = @ptrCast(*zigmui.Context, @alignCast(@alignOf(zigmui.Context), c.glfwGetWindowUserPointer(window)));
    if (action == c.GLFW_PRESS) {
        ctx.input.set_mousedown(switch (button) {
            c.GLFW_MOUSE_BUTTON_LEFT => zigmui.Input.MOUSE_BUTTON.LEFT,
            c.GLFW_MOUSE_BUTTON_RIGHT => zigmui.Input.MOUSE_BUTTON.RIGHT,
            c.GLFW_MOUSE_BUTTON_MIDDLE => zigmui.Input.MOUSE_BUTTON.MIDDLE,
            else => return,
        });
    } else if (action == c.GLFW_RELEASE) {
        ctx.input.set_mouseup(switch (button) {
            c.GLFW_MOUSE_BUTTON_LEFT => zigmui.Input.MOUSE_BUTTON.LEFT,
            c.GLFW_MOUSE_BUTTON_RIGHT => zigmui.Input.MOUSE_BUTTON.RIGHT,
            c.GLFW_MOUSE_BUTTON_MIDDLE => zigmui.Input.MOUSE_BUTTON.MIDDLE,
            else => return,
        });
    }
}

fn scroll_callback(window: ?*c.GLFWwindow, xoffset: f64, yoffset: f64) callconv(.C) void {
    const ctx = @ptrCast(*zigmui.Context, @alignCast(@alignOf(zigmui.Context), c.glfwGetWindowUserPointer(window)));
    ctx.input.set_scroll(@floatToInt(c_int, xoffset), @floatToInt(c_int, yoffset * -30));
}

var key_map = [_]zigmui.KEY{@intToEnum(zigmui.KEY, 0)} ** 1024;

fn key_callback(window: ?*c.GLFWwindow, key: c_int, scancode: c_int, action: c_int, mods: c_int) callconv(.C) void {
    _ = scancode;
    _ = mods;
    const ctx = @ptrCast(*zigmui.Context, @alignCast(@alignOf(zigmui.Context), c.glfwGetWindowUserPointer(window)));
    const ch = key_map[@intCast(usize, key)];
    switch (action) {
        c.GLFW_PRESS => {
            ctx.input.set_keydown(ch);
        },
        c.GLFW_RELEASE => {
            ctx.input.set_keyup(ch);
        },
        else => {},
    }
}

fn character_callback(window: ?*c.GLFWwindow, codepoint: c_uint) callconv(.C) void {
    const src = [1]u16{@intCast(u16, codepoint)};
    var buf: [4]u8 = undefined;
    const size = std.unicode.utf16leToUtf8(&buf, &src) catch unreachable;

    const ctx = @ptrCast(*zigmui.Context, @alignCast(@alignOf(zigmui.Context), c.glfwGetWindowUserPointer(window)));
    ctx.input.push_text(buf[0..size]);
}

pub fn main() anyerror!void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();
    defer std.debug.assert(!gpa.deinit());

    std.debug.assert(c.glfwInit() != 0);
    defer c.glfwTerminate();

    const window = c.glfwCreateWindow(1200, 1000, "Hello zig World", null, null) orelse {
        @panic("glfwCreateWindow");
    };
    defer c.glfwDestroyWindow(window);

    // Make the window's context current
    c.glfwMakeContextCurrent(window);

    c.MUI_RENDERER_init(c.glfwGetProcAddress);

    // init microui
    var ctx = try allocator.create(zigmui.Context);
    defer allocator.destroy(ctx);
    ctx.* = zigmui.Context{};
    var style = &ctx.command_drawer.style;
    style.text_width_callback = &text_width;
    style.text_height_callback = &text_height;
    c.glfwSetWindowUserPointer(window, ctx);

    _ = c.glfwSetCursorPosCallback(window, cursor_position_callback);
    _ = c.glfwSetMouseButtonCallback(window, mouse_button_callback);
    _ = c.glfwSetScrollCallback(window, scroll_callback);
    _ = c.glfwSetKeyCallback(window, key_callback);
    //   key_map[GLFW_KEY_LEFT_SHIFT] = MU_KEY_SHIFT;
    //   key_map[GLFW_KEY_RIGHT_SHIFT] = MU_KEY_SHIFT;
    //   key_map[GLFW_KEY_LEFT_CONTROL] = MU_KEY_CTRL;
    //   key_map[GLFW_KEY_RIGHT_CONTROL] = MU_KEY_CTRL;
    //   key_map[GLFW_KEY_LEFT_ALT] = MU_KEY_ALT;
    //   key_map[GLFW_KEY_RIGHT_ALT] = MU_KEY_ALT;
    //   key_map[GLFW_KEY_ENTER] = MU_KEY_RETURN;
    //   key_map[GLFW_KEY_BACKSPACE] = MU_KEY_BACKSPACE;
    _ = c.glfwSetCharCallback(window, character_callback);

    var bg = [3]f32{
        90, 95, 100,
    };

    // Loop until the user closes the window
    while (c.glfwWindowShouldClose(window) == 0) {
        // Poll for and process events
        c.glfwPollEvents();

        var width: c_int = undefined;
        var height: c_int = undefined;
        c.glfwGetFramebufferSize(window, &width, &height);

        var frame: c.UIRenderFrame = undefined;
        try ui.process_frame(ctx, &bg, &frame);
        c.MUI_RENDERER_render(width, height, &bg[0], &frame);

        // Swap front and back buffers
        c.glfwSwapBuffers(window);
    }

}
