const std = @import("std");
const c = @import("c");

extern fn ENGINE_init(p: ?*const anyopaque) callconv(.C) void;
extern fn ENGINE_mousemove(x: c_int, y: c_int) callconv(.C) void;
extern fn ENGINE_mousebutton_press(button: c_int) callconv(.C) void;
extern fn ENGINE_mousebutton_release(button: c_int) callconv(.C) void;
extern fn ENGINE_mousewheel(x: c_int, y: c_int) callconv(.C) void;
extern fn ENGINE_key_press(key: c_int) callconv(.C) void;
extern fn ENGINE_key_release(key: c_int) callconv(.C) void;
extern fn ENGINE_unicode(cp: c_uint) callconv(.C) void;
extern fn ENGINE_render(width: c_int, height: c_int) callconv(.C) void;

fn cursor_position_callback(_: ?*c.GLFWwindow, xpos: f64, ypos: f64) callconv(.C) void {
    ENGINE_mousemove(@floatToInt(c_int, xpos), @floatToInt(c_int, ypos));
}

fn mouse_button_callback(
    _: ?*c.GLFWwindow,
    button: c_int,
    action: c_int,
    _: c_int,
) callconv(.C) void {
    if (action == c.GLFW_PRESS) {
        ENGINE_mousebutton_press(switch (button) {
            c.GLFW_MOUSE_BUTTON_LEFT => 0,
            c.GLFW_MOUSE_BUTTON_RIGHT => 1,
            c.GLFW_MOUSE_BUTTON_MIDDLE => 2,
            else => return,
        });
    } else if (action == c.GLFW_RELEASE) {
        ENGINE_mousebutton_release(switch (button) {
            c.GLFW_MOUSE_BUTTON_LEFT => 0,
            c.GLFW_MOUSE_BUTTON_RIGHT => 1,
            c.GLFW_MOUSE_BUTTON_MIDDLE => 2,
            else => return,
        });
    }
}

fn scroll_callback(_: ?*c.GLFWwindow, xoffset: f64, yoffset: f64) callconv(.C) void {
    ENGINE_mousewheel(@floatToInt(c_int, xoffset), @floatToInt(c_int, yoffset));
}

fn key_callback(_: ?*c.GLFWwindow, key: c_int, scancode: c_int, action: c_int, mods: c_int) callconv(.C) void {
    _ = scancode;
    _ = mods;
    switch (action) {
        c.GLFW_PRESS => {
            ENGINE_key_press(key);
        },
        c.GLFW_RELEASE => {
            ENGINE_key_release(key);
        },
        else => {},
    }
}

fn character_callback(_: ?*c.GLFWwindow, codepoint: c_uint) callconv(.C) void {
    ENGINE_unicode(codepoint);
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

    ENGINE_init(c.glfwGetProcAddress);

    _ = c.glfwSetCursorPosCallback(window, cursor_position_callback);
    _ = c.glfwSetMouseButtonCallback(window, mouse_button_callback);
    _ = c.glfwSetScrollCallback(window, scroll_callback);
    _ = c.glfwSetKeyCallback(window, key_callback);
    _ = c.glfwSetCharCallback(window, character_callback);

    // Loop until the user closes the window
    while (c.glfwWindowShouldClose(window) == 0) {
        // Poll for and process events
        c.glfwPollEvents();

        var width: c_int = undefined;
        var height: c_int = undefined;
        c.glfwGetFramebufferSize(window, &width, &height);

        ENGINE_render(width, height);

        // Swap front and back buffers
        c.glfwSwapBuffers(window);
    }
}
