const std = @import("std");
const c = @import("c");
const ui = @import("./ui.zig");
const zigmui = @import("zigmui");

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
    const ctx = @ptrCast(*zigmui.Context, @alignCast(@alignOf(zigmui.Context), c.glfwGetWindowUserPointer(window)));
    mouse_x = @floatToInt(c_int, xpos);
    mouse_y = @floatToInt(c_int, ypos);
    ctx.input.input_mousemove(mouse_x, mouse_y);
}

fn mouse_button_callback(
    window: ?*c.GLFWwindow,
    button: c_int,
    action: c_int,
    _: c_int,
) callconv(.C) void {
    const ctx = @ptrCast(*zigmui.Context, @alignCast(@alignOf(zigmui.Context), c.glfwGetWindowUserPointer(window)));
    if (action == c.GLFW_PRESS) {
        ctx.input.input_mousedown(switch (button) {
            c.GLFW_MOUSE_BUTTON_LEFT => zigmui.Input.MOUSE_BUTTON.LEFT,
            c.GLFW_MOUSE_BUTTON_RIGHT => zigmui.Input.MOUSE_BUTTON.RIGHT,
            c.GLFW_MOUSE_BUTTON_MIDDLE => zigmui.Input.MOUSE_BUTTON.MIDDLE,
            else => return,
        });
    } else if (action == c.GLFW_RELEASE) {
        ctx.input.input_mouseup(switch (button) {
            c.GLFW_MOUSE_BUTTON_LEFT => zigmui.Input.MOUSE_BUTTON.LEFT,
            c.GLFW_MOUSE_BUTTON_RIGHT => zigmui.Input.MOUSE_BUTTON.RIGHT,
            c.GLFW_MOUSE_BUTTON_MIDDLE => zigmui.Input.MOUSE_BUTTON.MIDDLE,
            else => return,
        });
    }
}

fn scroll_callback(window: ?*c.GLFWwindow, xoffset: f64, yoffset: f64) callconv(.C) void {
    const ctx = @ptrCast(*zigmui.Context, @alignCast(@alignOf(zigmui.Context), c.glfwGetWindowUserPointer(window)));
    ctx.input.input_scroll(@floatToInt(c_int, xoffset), @floatToInt(c_int, yoffset * -30));
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

    c.r_init();

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
        c.render(width, height, &bg[0], &frame);

        // Swap front and back buffers
        c.glfwSwapBuffers(window);
    }
}
