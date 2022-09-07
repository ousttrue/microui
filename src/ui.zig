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
    const res = zigmui.widgets.slider(
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

var logbuf: [64000]u8 = undefined;
var logbuf_pos: usize = 0;
var logbuf_updated: bool = false;

fn write_log(text: []const u8) void {
    @memcpy(
        @ptrCast([*]u8, &logbuf[0]) + logbuf_pos,
        @ptrCast([*]const u8, &text[0]),
        text.len,
    );
    logbuf_pos += text.len;
    logbuf_updated = true;
    logbuf[logbuf_pos] = '\n';
    logbuf_pos += 1;
}

fn log_window(ctx: *zigmui.Context) void {
    if (zigmui.widgets.begin_window(ctx, "Log Window", .{ .x = 350, .y = 40, .w = 300, .h = 200 }, .NONE)) |_| {

        // output text panel
        ctx.layout.stack.back().row(&.{-1}, -25);
        zigmui.widgets.begin_panel(ctx, "Log Output", .{});
        const panel = ctx.container.current_container();
        _ = panel;
        ctx.layout.stack.back().row(&.{-1}, -1);
        zigmui.widgets.textarea(ctx, logbuf[0..logbuf_pos]);
        zigmui.widgets.end_panel(ctx);
        if (logbuf_updated) {
            // scroll to end
            // panel.scroll.y = panel.content_size.y;
            // logbuf_updated = false;
        }

        //     /* input textbox + submit button */
        //     static char buf[128];
        //     int submitted = 0;
        //     {
        //       int widths[] = {-70, -1};
        //       ctx.layout.stack.back().row(2, widths, 0);
        //     }
        //     if (zigmui.widgets.textbox(ctx, buf, sizeof(buf)) & MU_RES_SUBMIT) {
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
                write_log("Pressed button 1");
            }
            if (zigmui.widgets.button(ctx, .{ .text = "Button 2" }, .{}) != .NONE) {
                write_log("Pressed button 2");
            }
            zigmui.widgets.label(ctx, "Test buttons 2:");
            if (zigmui.widgets.button(ctx, .{ .text = "Button 3" }, .{}) != .NONE) {
                write_log("Pressed button 3");
            }
            if (zigmui.widgets.button(ctx, .{ .text = "Popup" }, .{}) != .NONE) {
                // zigmui.widgets.open_popup(ctx, "Test Popup");
            }
            // if (zigmui.widgets.begin_popup(ctx, "Test Popup")) {
            //     zigmui.widgets.button(ctx, "Hello");
            //     zigmui.widgets.button(ctx, "World");
            //     zigmui.widgets.end_popup(ctx);
            // }
        }

        // tree
        if (zigmui.widgets.header(ctx, "Tree and Text", .{ .opt = .EXPANDED }) != .NONE) {
            ctx.layout.stack.back().row(&.{ 140, -1 }, 0);
            const style = &ctx.command_drawer.style;
            {
                var layout = ctx.layout.begin_column(style);
                if (zigmui.widgets.begin_treenode(ctx, "Test 1", .{}) != .NONE) {
                    if (zigmui.widgets.begin_treenode(ctx, "Test 1a", .{}) != .NONE) {
                        zigmui.widgets.label(ctx, "Hello");
                        zigmui.widgets.label(ctx, "world");
                        zigmui.widgets.end_treenode(ctx);
                    }
                    if (zigmui.widgets.begin_treenode(ctx, "Test 1b", .{}) != .NONE) {
                        if (zigmui.widgets.button(ctx, .{ .text = "Button 1" }, .{}) != .NONE) {
                            write_log("Pressed button 1");
                        }
                        if (zigmui.widgets.button(ctx, .{ .text = "Button 2" }, .{}) != .NONE) {
                            write_log("Pressed button 2");
                        }
                        zigmui.widgets.end_treenode(ctx);
                    }
                    zigmui.widgets.end_treenode(ctx);
                }

                if (zigmui.widgets.begin_treenode(ctx, "Test 2", .{}) != .NONE) {
                    layout.row(&.{ 54, 54 }, 0);
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
                    zigmui.widgets.end_treenode(ctx);
                }

                if (zigmui.widgets.begin_treenode(ctx, "Test 3", .{}) != .NONE) {
                    const S = struct {
                        var checks = [_]bool{ true, false, true };
                    };
                    _ = zigmui.widgets.checkbox(ctx, "Checkbox 1", &S.checks[0]);
                    _ = zigmui.widgets.checkbox(ctx, "Checkbox 2", &S.checks[1]);
                    _ = zigmui.widgets.checkbox(ctx, "Checkbox 3", &S.checks[2]);
                    zigmui.widgets.end_treenode(ctx);
                }

                ctx.layout.end_column();
            }

            {
                var layout = ctx.layout.begin_column(style);
                layout.row(&.{-1}, 0);
                zigmui.widgets.textarea(ctx,
                    \\Lorem ipsum dolor sit amet, consectetur adipiscing
                    \\elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus
                    \\ipsum, eu varius magna felis a nulla.
                );
                ctx.layout.end_column();
            }
        }

        // background color sliders
        if (zigmui.widgets.header(ctx, "Background Color", .{ .opt = .EXPANDED }) != .NONE) {
            ctx.layout.stack.back().row(&.{ -78, -1 }, 74);

            // sliders
            const style = &ctx.command_drawer.style;
            var layout = ctx.layout.begin_column(style);
            layout.row(&.{ 46, -1 }, 0);
            zigmui.widgets.label(ctx, "Red:");
            _ = zigmui.widgets.slider(ctx, &bg[0], 0, 255, 0, null, .NONE);
            zigmui.widgets.label(ctx, "Green:");
            _ = zigmui.widgets.slider(ctx, &bg[1], 0, 255, 0, null, .NONE);
            zigmui.widgets.label(ctx, "Blue:");
            _ = zigmui.widgets.slider(ctx, &bg[2], 0, 255, 0, null, .NONE);
            ctx.layout.end_column();

            // color preview
            const rect = ctx.layout.back().next(style);
            ctx.command_drawer.draw_rect(rect, .{
                .r = @floatToInt(u8, bg[0]),
                .g = @floatToInt(u8, bg[1]),
                .b = @floatToInt(u8, bg[2]),
                .a = 255,
            });
            var buf: [32]u8 = undefined;
            const slice = std.fmt.bufPrint(&buf, "#{x}{x}{x}", .{
                @floatToInt(u8, bg[0]),
                @floatToInt(u8, bg[1]),
                @floatToInt(u8, bg[2]),
            }) catch unreachable;
            ctx.command_drawer.draw_control_text(slice, rect, .TEXT, .ALIGNCENTER, false);
        }

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
