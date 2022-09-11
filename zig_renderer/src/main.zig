const std = @import("std");
const builtin = @import("builtin");
const zigmui = @import("zigmui");
const logger = std.log.scoped(.zig_renderer);
const atlas = @import("./atlas.zig");
const Texture = @import("./Texture.zig");
const Renderer = @import("./Renderer.zig");
const ui = @import("./ui.zig");

pub extern fn console_logger(level: c_int, ptr: *const u8, size: c_int) void;

fn extern_write(level: c_int, m: []const u8) error{}!usize {
    if (m.len > 0) {
        console_logger(level, &m[0], @intCast(c_int, m.len));
    }
    return m.len;
}

pub fn log(
    comptime message_level: std.log.Level,
    comptime scope: @Type(.EnumLiteral),
    comptime format: []const u8,
    args: anytype,
) void {
    if (builtin.target.cpu.arch == .wasm32) {
        const level = switch (message_level) {
            .err => 0,
            .warn => 1,
            .info => 2,
            .debug => 3,
        };
        const w = std.io.Writer(c_int, error{}, extern_write){
            .context = level,
        };
        w.print(format, args) catch |err| {
            const err_name = @errorName(err);
            extern_write(0, err_name) catch unreachable;
        };
        _ = extern_write(level, "\n") catch unreachable;
    } else {
        std.log.defaultLog(message_level, scope, format, args);
    }
}

fn text_width(_: ?*anyopaque, text: []const u8) u32 {
    if (text.len == 0) {
        return 0;
    }
    return atlas.get_text_width(text);
}

fn text_height(_: ?*anyopaque) u32 {
    return atlas.get_text_height();
}

fn ptrAlignCast(comptime T: type, p: *const anyopaque) T {
    @setRuntimeSafety(false);
    const info = @typeInfo(T);
    return @ptrCast(T, @alignCast(info.Pointer.alignment, p));
}

var g_renderer: ?Renderer = null;
var g_ctx: ?*zigmui.Context = null;
var gpa: std.heap.GeneralPurposeAllocator(.{}) = undefined;
var allocator: std.mem.Allocator = undefined;

export fn ENGINE_init(p: *const anyopaque) callconv(.C) void {
    std.debug.assert(g_renderer == null);
    g_renderer = Renderer.init(p, atlas.width, atlas.height, atlas.data);
    gpa = std.heap.GeneralPurposeAllocator(.{}){};
    allocator = gpa.allocator();

    // init microui
    var ctx = allocator.create(zigmui.Context) catch unreachable;
    g_ctx = ctx;
    ctx.* = zigmui.Context{};
    var style = &ctx.command_drawer.style;
    style.text_width_callback = &text_width;
    style.text_height_callback = &text_height;
    // gl.fwSetWindowUserPointer(window, ctx);
}

export fn ENGINE_deinit() callconv(.C) void {
    if (g_ctx) |ctx| {
        allocator.destroy(ctx);
    }
    std.debug.assert(!gpa.deinit());
}

export fn ENGINE_mousemove(x: c_int, y: c_int) callconv(.C) void {
    const ctx = g_ctx orelse return;
    ctx.input.set_mousemove(x, y);
}
export fn ENGINE_mousebutton_press(button: c_int) callconv(.C) void {
    const ctx = g_ctx orelse return;
    ctx.input.set_mousedown(switch (button) {
        0 => zigmui.Input.MOUSE_BUTTON.LEFT,
        1 => zigmui.Input.MOUSE_BUTTON.RIGHT,
        2 => zigmui.Input.MOUSE_BUTTON.MIDDLE,
        else => return,
    });
}
export fn ENGINE_mousebutton_release(button: c_int) callconv(.C) void {
    const ctx = g_ctx orelse return;
    ctx.input.set_mouseup(switch (button) {
        0 => zigmui.Input.MOUSE_BUTTON.LEFT,
        1 => zigmui.Input.MOUSE_BUTTON.RIGHT,
        2 => zigmui.Input.MOUSE_BUTTON.MIDDLE,
        else => return,
    });
}
export fn ENGINE_mousewheel(x: c_int, y: c_int) callconv(.C) void {
    const ctx = g_ctx orelse return;
    ctx.input.set_scroll(x, y);
}

fn keyMap(ch: c_int) zigmui.Input.KEY {
    return switch (ch) {
        // GLFW_KEY_LEFT_SHIFT
        340 => .SHIFT,
        // GLFW_KEY_RIGHT_SHIFT
        344 => .SHIFT,
        // GLFW_KEY_LEFT_CONTROL
        341 => .CTRL,
        // GLFW_KEY_RIGHT_CONTROL
        345 => .CTRL,
        // GLFW_KEY_LEFT_ALT
        342 => .ALT,
        // GLFW_KEY_RIGHT_ALT
        346 => .ALT,
        // GLFW_KEY_ENTER
        257 => .RETURN,
        // GLFW_KEY_BACKSPACE
        259 => .BACKSPACE,
        else => .NONE,
    };
}
export fn ENGINE_key_press(ch: c_int) callconv(.C) void {
    const ctx = g_ctx orelse return;
    ctx.input.set_keydown(keyMap(ch));
}
export fn ENGINE_key_release(ch: c_int) callconv(.C) void {
    const ctx = g_ctx orelse return;
    ctx.input.set_keyup(keyMap(ch));
}
export fn ENGINE_unicode(cp: c_uint) callconv(.C) void {
    const ctx = g_ctx orelse return;
    var buf: [4]u8 = undefined;
    if (std.unicode.utf8Encode(@intCast(u21, cp), &buf)) |len| {
        ctx.input.push_text(buf[0..len]);
    } else |_| {}
}

var bg = [3]f32{
    90,
    95,
    100,
};
export fn ENGINE_render(width: c_int, height: c_int) callconv(.C) void {
    const ctx = g_ctx orelse {
        return;
    };
    var command: zigmui.RenderFrame = undefined;
    try ui.process_frame(ctx, &bg, &command);

    if (g_renderer) |*r| {
        r.begin(width, height, bg[0..3]);

        for (command.slice()) |it| {
            var p = command.get(it.head);
            var end = command.get(it.tail);
            while (p != end) {
                const command_type = @intToEnum(zigmui.COMMAND, ptrAlignCast(*const c_int, p).*);
                switch (command_type) {
                    .CLIP => {
                        const cmd = ptrAlignCast(*const zigmui.ClipCommand, p + 4);
                        r.set_clip_rect(cmd.rect);
                        p += (4 + @sizeOf(zigmui.ClipCommand));
                    },
                    .RECT => {
                        const cmd = ptrAlignCast(*const zigmui.RectCommand, p + 4);
                        r.draw_rect(cmd.rect, cmd.color);
                        p += (4 + @sizeOf(zigmui.RectCommand));
                    },
                    .TEXT => {
                        const cmd = ptrAlignCast(*const zigmui.TextCommand, p + 4);
                        const begin = 4 + @sizeOf(zigmui.TextCommand);
                        const text = p[begin .. begin + cmd.length];
                        r.draw_text(text, cmd.pos, cmd.color);
                        p += (4 + @sizeOf(zigmui.TextCommand) + cmd.length);
                    },
                    .ICON => {
                        const cmd = ptrAlignCast(*const zigmui.IconCommand, p + 4);
                        r.draw_icon(@intCast(u32, cmd.id), cmd.rect, cmd.color);
                        p += (4 + @sizeOf(zigmui.IconCommand));
                    },
                }
            }
        }

        r.flush();
    }
}
