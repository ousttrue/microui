const std = @import("std");
const c = @import("c");

export fn r_init() callconv(.C) void {}

export fn r_get_text_width(text: [*]const u8, len: c_int) callconv(.C) c_int {
    _ = text;
    _ = len;
    return 0;
}

export fn r_get_text_height() callconv(.C) c_int {
    return 0;
}

export fn render(width: c_int, height: c_int, bg: [*]const f32, command: *const anyopaque) void {
    _ = width;
    _ = height;
    _ = bg;
    _ = command;

    c.glViewport(0, 0, width, height);
    c.glScissor(0, 0, width, height);
    //   r_clear(UIColor32(bg[0], bg[1], bg[2], 255));
    c.glClearColor(0.2, 0.3, 0.4, 1);
    c.glClear(c.GL_COLOR_BUFFER_BIT);
}
