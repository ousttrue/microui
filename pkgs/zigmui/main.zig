const std = @import("std");
const c = @import("c");

pub const Context = @import("./Context.zig");

pub fn input_mousemove(ctx: *Context, mouse_x: i32, mouse_y: i32) void {
    _ = ctx;
    _ = mouse_x;
    _ = mouse_y;
}

pub fn input_mousedown(ctx: *Context, mouse_button: u32) void {
    _ = ctx;
    _ = mouse_button;
}

pub fn input_mouseup(ctx: *Context, mouse_button: u32) void {
    _ = ctx;
    _ = mouse_button;
}

pub fn input_scroll(ctx: *Context, x: i32, y: i32) void {
    _ = ctx;
    _ = x;
    _ = y;
}
