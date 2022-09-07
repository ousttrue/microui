const std = @import("std");
const c = @import("c");
const zigmui = @import("zigmui");

const colors = [_]struct {
    label: []const u8,
    idx: c_int,
}{
    .{ .label = "text:", .idx = c.MU_STYLE_TEXT },
    .{ .label = "border:", .idx = c.MU_STYLE_BORDER },
    .{ .label = "windowbg:", .idx = c.MU_STYLE_WINDOWBG },
    .{ .label = "titlebg:", .idx = c.MU_STYLE_TITLEBG },
    .{ .label = "titletext:", .idx = c.MU_STYLE_TITLETEXT },
    .{ .label = "panelbg:", .idx = c.MU_STYLE_PANELBG },
    .{ .label = "button:", .idx = c.MU_STYLE_BUTTON },
    .{ .label = "buttonhover:", .idx = c.MU_STYLE_BUTTONHOVER },
    .{ .label = "buttonfocus:", .idx = c.MU_STYLE_BUTTONFOCUS },
    .{ .label = "base:", .idx = c.MU_STYLE_BASE },
    .{ .label = "basehover:", .idx = c.MU_STYLE_BASEHOVER },
    .{ .label = "basefocus:", .idx = c.MU_STYLE_BASEFOCUS },
    .{ .label = "scrollbase:", .idx = c.MU_STYLE_SCROLLBASE },
    .{ .label = "scrollthumb:", .idx = c.MU_STYLE_SCROLLTHUMB },
};

pub fn uint8_slider(ctx: *zigmui.Context, value: *u8, low: i32, high: i32) zigmui.RES {
    const id = ctx.hash.from_value(value);
    ctx.hash.stack.push(id);
    var tmp = @intToFloat(f32, value.*);
    const res = zigmui.widgets.slider_ex(
        ctx,
        &tmp,
        @intToFloat(f32, low),
        @intToFloat(f32, high),
        0,
        "{d:.0}",
        .ALIGNCENTER,
    );
    value.* = @floatToInt(u8, tmp);
    ctx.hash.stack.pop();
    return res;
}

fn style_window(ctx: *zigmui.Context) void {
    if (zigmui.widgets.begin_window(ctx, "Style Editor", .{ .x = 350, .y = 250, .w = 300, .h = 240 }, .NONE)) |_| {
        const sw = @floatToInt(i32, @intToFloat(f32, ctx.container.current_container().body.w) * 0.14);
        {
            const widths = [_]i32{ 80, sw, sw, sw, sw, -1 };
            ctx.layout.stack.back().row(&widths, 0);
        }
        const style = &ctx.command_drawer.style;
        for (colors) |color, i| {
            zigmui.widgets.label(ctx, color.label);
            const style_color = &style.colors[i];
            _ = uint8_slider(ctx, &style_color.r, 0, 255);
            _ = uint8_slider(ctx, &style_color.g, 0, 255);
            _ = uint8_slider(ctx, &style_color.b, 0, 255);
            _ = uint8_slider(ctx, &style_color.a, 0, 255);
            ctx.command_drawer.draw_rect(ctx.layout.stack.back().next(style), style.colors[i]);
        }
        zigmui.widgets.end_window(ctx);
    }
}

fn log_window(ctx: *zigmui.Context) void {
    if (zigmui.widgets.begin_window(ctx, "Log Window", .{ .x = 350, .y = 40, .w = 300, .h = 200 }, .NONE)) |_| {

        //     /* output text panel */
        //     {
        //       int widths[] = {-1};
        //       ctx.layout.stack.back().row(1, widths, -25);
        //     }
        //     mu_begin_panel(ctx, "Log Output");
        //     auto panel = mu_get_current_container(ctx);
        //     {
        //       int widths[] = {-1};
        //       ctx.layout.stack.back().row(1, widths, -1);
        //     }
        //     mu_text(ctx, logbuf);
        //     mu_end_panel(ctx);
        //     if (logbuf_updated) {
        //       panel.scroll.y = panel.content_size.y;
        //       logbuf_updated = 0;
        //     }

        //     /* input textbox + submit button */
        //     static char buf[128];
        //     int submitted = 0;
        //     {
        //       int widths[] = {-70, -1};
        //       ctx.layout.stack.back().row(2, widths, 0);
        //     }
        //     if (mu_textbox(ctx, buf, sizeof(buf)) & MU_RES_SUBMIT) {
        //       ctx.set_focus(ctx.last_id);
        //       submitted = 1;
        //     }
        //     if (zigmui.widgets.button(ctx, "Submit")) {
        //       submitted = 1;
        //     }
        //     if (submitted) {
        //       write_log(buf);
        //       buf[0] = '\0';
        //     }

        zigmui.widgets.end_window(ctx);
    }
}

fn test_window(ctx: *zigmui.Context, bg: [*]f32) void {
    _ = bg;
    if (zigmui.widgets.begin_window(ctx, "Demo Window", .{ .x = 40, .y = 40, .w = 300, .h = 450 }, .NONE)) |_| {
        const win = ctx.container.current_container();
        win.rect.w = std.math.max(win.rect.w, 240);
        win.rect.h = std.math.max(win.rect.h, 300);

        // window info
        if (zigmui.widgets.header(ctx, "Window Info", .{}) != .NONE) {
            var buf: [64]u8 = undefined;
            ctx.layout.stack.back().row(&.{ 54, -1 }, 0);
            zigmui.widgets.label(ctx, "Position:");
            {
                const slice = std.fmt.bufPrint(&buf, "{}, {}", .{ win.rect.x, win.rect.y }) catch unreachable;
                zigmui.widgets.label(ctx, slice);
            }
            zigmui.widgets.label(ctx, "Size:");
            {
                const slice = std.fmt.bufPrint(&buf, "{}, {}", .{ win.rect.w, win.rect.h }) catch unreachable;
                zigmui.widgets.label(ctx, slice);
            }
        }

        // labels + buttons
        if (zigmui.widgets.header(ctx, "Test Buttons", .{ .opt = .EXPANDED }) != .NONE) {
            ctx.layout.stack.back().row(&.{ 86, -110, -1 }, 0);
            zigmui.widgets.label(ctx, "Test buttons 1:");
            if (zigmui.widgets.button(ctx, .{ .text = "Button 1" }, .{}) != .NONE) {
                // write_log("Pressed button 1");
            }
            if (zigmui.widgets.button(ctx, .{ .text = "Button 2" }, .{}) != .NONE) {
                // write_log("Pressed button 2");
            }
            zigmui.widgets.label(ctx, "Test buttons 2:");
            if (zigmui.widgets.button(ctx, .{ .text = "Button 3" }, .{}) != .NONE) {
                // write_log("Pressed button 3");
            }
            if (zigmui.widgets.button(ctx, .{ .text = "Popup" }, .{}) != .NONE) {
                // mu_open_popup(ctx, "Test Popup");
            }
            // if (mu_begin_popup(ctx, "Test Popup")) {
            //     zigmui.widgets.button(ctx, "Hello");
            //     zigmui.widgets.button(ctx, "World");
            //     mu_end_popup(ctx);
            // }
        }

        // /* tree */
        // if (mu_header_ex(ctx, "Tree and Text", MU_OPT_EXPANDED)) {
        //   {
        //     int widths[] = {140, -1};
        //     ctx.layout.stack.back().row(2, widths, 0);
        //   }
        //   mu_layout_begin_column(ctx);
        //   if (mu_begin_treenode(ctx, "Test 1")) {
        //     if (mu_begin_treenode(ctx, "Test 1a")) {
        //       zigmui.widgets.label(ctx, "Hello");
        //       zigmui.widgets.label(ctx, "world");
        //       mu_end_treenode(ctx);
        //     }
        //     if (mu_begin_treenode(ctx, "Test 1b")) {
        //       if (zigmui.widgets.button(ctx, "Button 1")) {
        //         write_log("Pressed button 1");
        //       }
        //       if (zigmui.widgets.button(ctx, "Button 2")) {
        //         write_log("Pressed button 2");
        //       }
        //       mu_end_treenode(ctx);
        //     }
        //     mu_end_treenode(ctx);
        //   }
        //   if (mu_begin_treenode(ctx, "Test 2")) {
        //     {
        //       int widths[] = {54, 54};
        //       ctx.layout.stack.back().row(2, widths, 0);
        //     }
        //     if (zigmui.widgets.button(ctx, "Button 3")) {
        //       write_log("Pressed button 3");
        //     }
        //     if (zigmui.widgets.button(ctx, "Button 4")) {
        //       write_log("Pressed button 4");
        //     }
        //     if (zigmui.widgets.button(ctx, "Button 5")) {
        //       write_log("Pressed button 5");
        //     }
        //     if (zigmui.widgets.button(ctx, "Button 6")) {
        //       write_log("Pressed button 6");
        //     }
        //     mu_end_treenode(ctx);
        //   }
        //   if (mu_begin_treenode(ctx, "Test 3")) {
        //     static int checks[3] = {1, 0, 1};
        //     mu_checkbox(ctx, "Checkbox 1", &checks[0]);
        //     mu_checkbox(ctx, "Checkbox 2", &checks[1]);
        //     mu_checkbox(ctx, "Checkbox 3", &checks[2]);
        //     mu_end_treenode(ctx);
        //   }
        //   mu_layout_end_column(ctx);

        //   mu_layout_begin_column(ctx);
        //   {
        //     int widths[] = {-1};
        //     ctx.layout.stack.back().row(1, widths, 0);
        //   }
        //   mu_text(
        //       ctx,
        //       "Lorem ipsum dolor sit amet, consectetur adipiscing "
        //       "elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus "
        //       "ipsum, eu varius magna felis a nulla.");
        //   mu_layout_end_column(ctx);
        // }

        // /* background color sliders */
        // if (mu_header_ex(ctx, "Background Color", MU_OPT_EXPANDED)) {
        //   {
        //     int widths[] = {-78, -1};
        //     ctx.layout.stack.back().row(2, widths, 74);
        //   }
        //   /* sliders */
        //   mu_layout_begin_column(ctx);
        //   {
        //     int widths[] = {46, -1};
        //     ctx.layout.stack.back().row(2, widths, 0);
        //   }
        //   zigmui.widgets.label(ctx, "Red:");
        //   mu_slider(ctx, &bg[0], 0, 255);
        //   zigmui.widgets.label(ctx, "Green:");
        //   mu_slider(ctx, &bg[1], 0, 255);
        //   zigmui.widgets.label(ctx, "Blue:");
        //   mu_slider(ctx, &bg[2], 0, 255);
        //   mu_layout_end_column(ctx);
        //   /* color preview */
        //   UIRect r = mu_layout_next(ctx);
        //   ctx.draw_rect(r, mu_Color(bg[0], bg[1], bg[2], 255));
        //   char buf[32];
        //   sprintf(buf, "#%02X%02X%02X", (int)bg[0], (int)bg[1], (int)bg[2]);
        //   mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
        // }

        zigmui.widgets.end_window(ctx);
    }
}

var command_groups: [1]c.struct_UICommandRange = undefined;
var command_buffer: [1024]u8 = undefined;

pub fn process_frame(ctx: *zigmui.Context, bg: [*]f32, frame: *c.UIRenderFrame) !void {
    ctx.begin();

    style_window(ctx);
    log_window(ctx);
    test_window(ctx, bg);

    try ctx.end(frame);
}
