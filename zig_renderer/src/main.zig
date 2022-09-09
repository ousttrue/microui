const std = @import("std");
const c = @import("c");
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

var g_renderer: Renderer = undefined;

export fn MUI_RENDERER_init(p: *const anyopaque) callconv(.C) void {
    g_renderer = Renderer.init();
    g_renderer.loadfunc(p);
    g_renderer.atlas_texture = Texture.init(atlas.width, atlas.height, atlas.data);
}

export fn MUI_RENDERER_render(width: c_int, height: c_int, bg: [*]const f32, command: *const anyopaque) void {
  g_renderer.begin(width, height, bg[0..3]);
  _ = command;

//   auto end = command->end();
//   for (auto it = command->begin(); it != end; ++it) {
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
//   }

  g_renderer.flush();
}
