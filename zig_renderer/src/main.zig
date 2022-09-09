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
    const info = @typeInfo(T);
    logger.debug("{}", .{info});
    return @ptrCast(T, @alignCast(info.Pointer.alignment, p));
}

export fn MUI_RENDERER_render(width: c_int, height: c_int, bg: [*]const f32, command: *const c.UIRenderFrame) void {
    if (g_renderer) |*r| {
        r.begin(width, height, bg[0..3]);

        for (command.command_groups[0..command.command_group_count]) |it| {
            var p = command.command_buffer + it.head;
            var end = command.command_buffer + it.tail;
            while (p != end) {
                const command_type = @ptrCast(*const c_int, @alignCast(4, p)).*;
                const size = switch (command_type) {
                    c.UI_COMMAND_CLIP =>
                    // const cmd = ptrAlignCast(*const c.UIClipCommand, p + 4);
                    // _ = cmd;
                    (4 + @sizeOf(c.UIClipCommand)),

                    c.UI_COMMAND_RECT =>
                    // const cmd = ptrAlignCast(*const c.UIRectCommand, p + 4);
                    // _ = cmd;
                    (4 + @sizeOf(c.UIRectCommand)),

                    c.UI_COMMAND_TEXT =>
                    // const cmd = ptrAlignCast(*const c.UITextCommand, p + 4);
                    // const cmd = @ptrCast(*const c.UITextCommand, @alignCast(8, p + 4));
                    // _ = cmd;
                    (4 + @sizeOf(c.UITextCommand) + @ptrCast(*const u32, @alignCast(4, p + 4 + 12)).*),
                    c.UI_COMMAND_ICON =>
                    // const cmd = ptrAlignCast(*const c.UIIconCommand, p + 4);
                    // _ = cmd;
                    (4 + @sizeOf(c.UIIconCommand)),
                    else => unreachable,
                };
                // logger.debug("{}, {}", .{command_type, size});
                p += size;
            }

            // logger.debug("end", .{});

            //     auto tail = command->command_buffer + it->tail;
            //     UICommandHeader *cmd = nullptr;
            //     for (auto p = command->command_buffer + it->head; p != tail;
            //          p = p + cmd->size()) {
            //       cmd = (UICommandHeader *)p;
            //       switch (cmd->command) {
            //       case UI_COMMAND_TEXT:
            //         g_renderer.draw_text(cmd->text()->begin(), cmd->text()->end(),
            //                              cmd->text()->pos, cmd->text()->color);
            //         break;
            //       case UI_COMMAND_RECT:
            //         g_renderer.draw_rect(cmd->rect()->rect, cmd->rect()->color);
            //         break;
            //       case UI_COMMAND_ICON:
            //         g_renderer.draw_icon(cmd->icon()->id, cmd->icon()->rect,
            //                              cmd->icon()->color);
            //         break;
            //       case UI_COMMAND_CLIP:
            //         g_renderer.set_clip_rect(cmd->clip()->rect);
            //         break;
            //       }
            //     }
        }

        r.flush();
    }
}
