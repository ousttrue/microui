const std = @import("std");
const c = @import("c");
const logger = std.log.scoped(.zig_renderer);
const atlas = @import("./atlas.zig");
const Texture = @import("./Texture.zig");
const Renderer = @import("./Renderer.zig");

export fn MUI_RENDERER_get_text_width(text: [*]const u8, len: c_int) callconv(.C) c_int {
    const slice = text[0..@intCast(usize, len)];
    return @intCast(c_int, atlas.get_text_width(slice));
}

export fn MUI_RENDERER_get_text_height() callconv(.C) c_int {
    return @intCast(c_int, atlas.get_text_height());
}

var g_renderer: ?Renderer = null;

export fn MUI_RENDERER_init(p: *const anyopaque) callconv(.C) void {
    std.debug.assert(g_renderer == null);
    g_renderer = Renderer.init(p, atlas.width, atlas.height, atlas.data);
}

fn ptrAlignCast(comptime T: type, p: *const anyopaque) T {
    @setRuntimeSafety(false);
    const info = @typeInfo(T);
    // logger.debug("{}", .{info});
    return @ptrCast(T, @alignCast(info.Pointer.alignment, p));
}

export fn MUI_RENDERER_render(width: c_int, height: c_int, bg: [*]const f32, command: *const c.UIRenderFrame) void {
    if (g_renderer) |*r| {
        r.begin(width, height, bg[0..3]);

        for (command.command_groups[0..command.command_group_count]) |it| {
            var p = command.command_buffer + it.head;
            var end = command.command_buffer + it.tail;
            while (p != end) {
                const command_type = ptrAlignCast(*const c_int, p).*;
                switch (command_type) {
                    c.UI_COMMAND_CLIP => {
                        const cmd = ptrAlignCast(*const c.UIClipCommand, p + 4);
                        r.set_clip_rect(cmd.rect);
                        p += (4 + @sizeOf(c.UIClipCommand));
                    },
                    c.UI_COMMAND_RECT => {
                        const cmd = ptrAlignCast(*const c.UIRectCommand, p + 4);
                        r.draw_rect(cmd.rect, cmd.color);
                        p += (4 + @sizeOf(c.UIRectCommand));
                    },
                    c.UI_COMMAND_TEXT => {
                        const cmd = ptrAlignCast(*const c.UITextCommand, p + 4);
                        const begin = 4 + @sizeOf(c.UITextCommand);
                        const text = p[begin .. begin + cmd.length];
                        r.draw_text(text, cmd.pos, cmd.color);
                        p += (4 + @sizeOf(c.UITextCommand) + cmd.length);
                    },
                    c.UI_COMMAND_ICON => {
                        const cmd = ptrAlignCast(*const c.UIIconCommand, p + 4);
                        r.draw_icon(@intCast(u32, cmd.id), cmd.rect, cmd.color);
                        p += (4 + @sizeOf(c.UIIconCommand));
                    },
                    else => unreachable,
                }
            }
        }

        r.flush();
    }
}
