const std = @import("std");
const c = @import("c");

fn text_width(_: ?*anyopaque, text: [*c]const u8, _len: c_int) callconv(.C) c_int {
    const len = if (_len == -1)
        @intCast(c_int, c.strlen(text))
    else
        _len;
    return c.r_get_text_width(text, len);
}

fn text_height(_: ?*anyopaque) callconv(.C) c_int {
    return c.r_get_text_height();
}

var mouse_x: c_int = 0;
var mouse_y: c_int = 0;
fn cursor_position_callback(window: ?*c.GLFWwindow, xpos: f64, ypos: f64) callconv(.C) void {
    const ctx = @ptrCast(*c.mu_Context, c.glfwGetWindowUserPointer(window));
    mouse_x = @floatToInt(c_int, xpos);
    mouse_y = @floatToInt(c_int, ypos);
    c.mu_input_mousemove(ctx, mouse_x, mouse_y);
}

fn mouse_button_callback(
    window: ?*c.GLFWwindow,
    button: c_int,
    action: c_int,
    _: c_int,
) callconv(.C) void {
    const ctx = @ptrCast(*c.mu_Context, c.glfwGetWindowUserPointer(window));
    if (action == c.GLFW_PRESS) {
        c.mu_input_mousedown(ctx, switch (button) {
            c.GLFW_MOUSE_BUTTON_LEFT => 1,
            c.GLFW_MOUSE_BUTTON_RIGHT => 2,
            c.GLFW_MOUSE_BUTTON_MIDDLE => 3,
            else => return,
        });
    } else if (action == c.GLFW_RELEASE) {
        c.mu_input_mouseup(ctx, switch (button) {
            c.GLFW_MOUSE_BUTTON_LEFT => 1,
            c.GLFW_MOUSE_BUTTON_RIGHT => 2,
            c.GLFW_MOUSE_BUTTON_MIDDLE => 3,
            else => return,
        });
    }
}

fn scroll_callback(window: ?*c.GLFWwindow, xoffset: f64, yoffset: f64) callconv(.C) void {
    const ctx = @ptrCast(*c.mu_Context, c.glfwGetWindowUserPointer(window));
    c.mu_input_scroll(ctx, @floatToInt(c_int, xoffset), @floatToInt(c_int, yoffset * -30));
}

pub fn main() anyerror!void {
    std.debug.assert(c.glfwInit() != 0);
    defer c.glfwTerminate();

    const window = c.glfwCreateWindow(1200, 1000, "Hello zig World", null, null) orelse {
        @panic("glfwCreateWindow");
    };
    defer c.glfwDestroyWindow(window);

    // Make the window's context current
    c.glfwMakeContextCurrent(window);

    c.r_init();

    // init microui
    const ctx = c.mu_new(text_width, text_height);
    c.glfwSetWindowUserPointer(window, ctx);

    _ = c.glfwSetCursorPosCallback(window, cursor_position_callback);
    _ = c.glfwSetMouseButtonCallback(window, mouse_button_callback);
    _ = c.glfwSetScrollCallback(window, scroll_callback);

    // Loop until the user closes the window
    while (c.glfwWindowShouldClose(window) == 0) {
        // Poll for and process events
        c.glfwPollEvents();

        var width: c_int = undefined;
        var height: c_int = undefined;
        c.glfwGetFramebufferSize(window, &width, &height);

        c.render(ctx, width, height);

        // Swap front and back buffers
        c.glfwSwapBuffers(window);
    }
}
