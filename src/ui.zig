const std = @import("std");
const c = @import("c");
const zigmui = @import("zigmui");

const colors = [_]struct {
    label: []const u8,
    idx: c_int,
}{
    .{ "text:", c.MU_COLOR_TEXT },
    .{ "border:", c.MU_COLOR_BORDER },
    .{ "windowbg:", c.MU_COLOR_WINDOWBG },
    .{ "titlebg:", c.MU_COLOR_TITLEBG },
    .{ "titletext:", c.MU_COLOR_TITLETEXT },
    .{ "panelbg:", c.MU_COLOR_PANELBG },
    .{ "button:", c.MU_COLOR_BUTTON },
    .{ "buttonhover:", c.MU_COLOR_BUTTONHOVER },
    .{ "buttonfocus:", c.MU_COLOR_BUTTONFOCUS },
    .{ "base:", c.MU_COLOR_BASE },
    .{ "basehover:", c.MU_COLOR_BASEHOVER },
    .{ "basefocus:", c.MU_COLOR_BASEFOCUS },
    .{ "scrollbase:", c.MU_COLOR_SCROLLBASE },
    .{ "scrollthumb:", c.MU_COLOR_SCROLLTHUMB },
};

fn style_window(ctx: *zigmui.Context) void {
    if (ctx.begin_window("Style Editor", .{ .x = 350, .y = 250, .w = 300, .h = 240 }, .NONE)) |_| {
        //     int sw = mu_get_current_container(ctx)->body.w * 0.14;
        //     {
        //       int widths[] = {80, sw, sw, sw, sw, -1};
        //       ctx->layout_stack.back().row(6, widths, 0);
        //     }
        //     for (int i = 0; colors[i].label; i++) {
        //       mu_label(ctx, colors[i].label);
        //       uint8_slider(ctx, &ctx->style->colors[i].r, 0, 255);
        //       uint8_slider(ctx, &ctx->style->colors[i].g, 0, 255);
        //       uint8_slider(ctx, &ctx->style->colors[i].b, 0, 255);
        //       uint8_slider(ctx, &ctx->style->colors[i].a, 0, 255);
        //       ctx->draw_rect(mu_layout_next(ctx), ctx->style->colors[i]);
        //     }
        ctx.end_window();
    }
}

fn log_window(ctx: *zigmui.Context) void {
    if (ctx.begin_window("Log Window", .{ .x = 350, .y = 40, .w = 300, .h = 200 }, .NONE)) |_| {

        //     /* output text panel */
        //     {
        //       int widths[] = {-1};
        //       ctx->layout_stack.back().row(1, widths, -25);
        //     }
        //     mu_begin_panel(ctx, "Log Output");
        //     auto panel = mu_get_current_container(ctx);
        //     {
        //       int widths[] = {-1};
        //       ctx->layout_stack.back().row(1, widths, -1);
        //     }
        //     mu_text(ctx, logbuf);
        //     mu_end_panel(ctx);
        //     if (logbuf_updated) {
        //       panel->scroll.y = panel->content_size.y;
        //       logbuf_updated = 0;
        //     }

        //     /* input textbox + submit button */
        //     static char buf[128];
        //     int submitted = 0;
        //     {
        //       int widths[] = {-70, -1};
        //       ctx->layout_stack.back().row(2, widths, 0);
        //     }
        //     if (mu_textbox(ctx, buf, sizeof(buf)) & MU_RES_SUBMIT) {
        //       ctx->set_focus(ctx->last_id);
        //       submitted = 1;
        //     }
        //     if (mu_button(ctx, "Submit")) {
        //       submitted = 1;
        //     }
        //     if (submitted) {
        //       write_log(buf);
        //       buf[0] = '\0';
        //     }

        ctx.end_window();
    }
}

fn test_window(ctx: *zigmui.Context, bg: [*]f32) void {
    _ = bg;
    if (ctx.begin_window("Demo Window", .{ .x = 40, .y = 40, .w = 300, .h = 450 }, .NONE)) |_| {
        // auto win = mu_get_current_container(ctx);
        // win->rect.w = mu_max(win->rect.w, 240);
        // win->rect.h = mu_max(win->rect.h, 300);

        // /* window info */
        // if (mu_header(ctx, "Window Info")) {
        //   auto win = mu_get_current_container(ctx);
        //   char buf[64];
        //   {
        //     int widths[] = {54, -1};
        //     ctx->layout_stack.back().row(2, widths, 0);
        //   }
        //   mu_label(ctx, "Position:");
        //   sprintf(buf, "%d, %d", win->rect.x, win->rect.y);
        //   mu_label(ctx, buf);
        //   mu_label(ctx, "Size:");
        //   sprintf(buf, "%d, %d", win->rect.w, win->rect.h);
        //   mu_label(ctx, buf);
        // }

        // /* labels + buttons */
        // if (mu_header_ex(ctx, "Test Buttons", MU_OPT_EXPANDED)) {
        //   {
        //     int widths[] = {86, -110, -1};
        //     ctx->layout_stack.back().row(3, widths, 0);
        //   }
        //   mu_label(ctx, "Test buttons 1:");
        //   if (mu_button(ctx, "Button 1")) {
        //     write_log("Pressed button 1");
        //   }
        //   if (mu_button(ctx, "Button 2")) {
        //     write_log("Pressed button 2");
        //   }
        //   mu_label(ctx, "Test buttons 2:");
        //   if (mu_button(ctx, "Button 3")) {
        //     write_log("Pressed button 3");
        //   }
        //   if (mu_button(ctx, "Popup")) {
        //     mu_open_popup(ctx, "Test Popup");
        //   }
        //   if (mu_begin_popup(ctx, "Test Popup")) {
        //     mu_button(ctx, "Hello");
        //     mu_button(ctx, "World");
        //     mu_end_popup(ctx);
        //   }
        // }

        // /* tree */
        // if (mu_header_ex(ctx, "Tree and Text", MU_OPT_EXPANDED)) {
        //   {
        //     int widths[] = {140, -1};
        //     ctx->layout_stack.back().row(2, widths, 0);
        //   }
        //   mu_layout_begin_column(ctx);
        //   if (mu_begin_treenode(ctx, "Test 1")) {
        //     if (mu_begin_treenode(ctx, "Test 1a")) {
        //       mu_label(ctx, "Hello");
        //       mu_label(ctx, "world");
        //       mu_end_treenode(ctx);
        //     }
        //     if (mu_begin_treenode(ctx, "Test 1b")) {
        //       if (mu_button(ctx, "Button 1")) {
        //         write_log("Pressed button 1");
        //       }
        //       if (mu_button(ctx, "Button 2")) {
        //         write_log("Pressed button 2");
        //       }
        //       mu_end_treenode(ctx);
        //     }
        //     mu_end_treenode(ctx);
        //   }
        //   if (mu_begin_treenode(ctx, "Test 2")) {
        //     {
        //       int widths[] = {54, 54};
        //       ctx->layout_stack.back().row(2, widths, 0);
        //     }
        //     if (mu_button(ctx, "Button 3")) {
        //       write_log("Pressed button 3");
        //     }
        //     if (mu_button(ctx, "Button 4")) {
        //       write_log("Pressed button 4");
        //     }
        //     if (mu_button(ctx, "Button 5")) {
        //       write_log("Pressed button 5");
        //     }
        //     if (mu_button(ctx, "Button 6")) {
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
        //     ctx->layout_stack.back().row(1, widths, 0);
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
        //     ctx->layout_stack.back().row(2, widths, 74);
        //   }
        //   /* sliders */
        //   mu_layout_begin_column(ctx);
        //   {
        //     int widths[] = {46, -1};
        //     ctx->layout_stack.back().row(2, widths, 0);
        //   }
        //   mu_label(ctx, "Red:");
        //   mu_slider(ctx, &bg[0], 0, 255);
        //   mu_label(ctx, "Green:");
        //   mu_slider(ctx, &bg[1], 0, 255);
        //   mu_label(ctx, "Blue:");
        //   mu_slider(ctx, &bg[2], 0, 255);
        //   mu_layout_end_column(ctx);
        //   /* color preview */
        //   UIRect r = mu_layout_next(ctx);
        //   ctx->draw_rect(r, mu_Color(bg[0], bg[1], bg[2], 255));
        //   char buf[32];
        //   sprintf(buf, "#%02X%02X%02X", (int)bg[0], (int)bg[1], (int)bg[2]);
        //   mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
        // }

        ctx.end_window();
    }
}

var command_groups: [1]c.struct_UICommandRange = undefined;
var command_buffer: [1024]u8 = undefined;

pub fn process_frame(ctx: *zigmui.Context, bg: [*]f32, frame: *c.UIRenderFrame) !void {
    _ = bg;

    ctx.begin();

    style_window(ctx);
    log_window(ctx);
    test_window(ctx, bg);

    try ctx.end(frame);
}
