const std = @import("std");
const zigmui = @import("zigmui");

const colors = [_]struct {
    label: []const u8,
    idx: zigmui.STYLE,
}{
    .{ .label = "text:", .idx = .TEXT },
    .{ .label = "border:", .idx = .BORDER },
    .{ .label = "windowbg:", .idx = .WINDOWBG },
    .{ .label = "titlebg:", .idx = .TITLEBG },
    .{ .label = "titletext:", .idx = .TITLETEXT },
    .{ .label = "panelbg:", .idx = .PANELBG },
    .{ .label = "button:", .idx = .BUTTON },
    .{ .label = "buttonhover:", .idx = .BUTTONHOVER },
    .{ .label = "buttonfocus:", .idx = .BUTTONFOCUS },
    .{ .label = "base:", .idx = .BASE },
    .{ .label = "basehover:", .idx = .BASEHOVER },
    .{ .label = "basefocus:", .idx = .BASEFOCUS },
    .{ .label = "scrollbase:", .idx = .SCROLLBASE },
    .{ .label = "scrollthumb:", .idx = .SCROLLTHUMB },
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
    if (zigmui.widgets.begin_window(ctx, "Style Editor", .{ .x = 350, .y = 250, .w = 300, .h = 240 }, .NONE)) {
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
    if (text.len > 0) {
        @memcpy(
            @ptrCast([*]u8, &logbuf[0]) + logbuf_pos,
            @ptrCast([*]const u8, &text[0]),
            text.len,
        );
        logbuf_pos += text.len;
    }
    logbuf_updated = true;
    logbuf[logbuf_pos] = '\n';
    logbuf_pos += 1;
}

fn log_window(ctx: *zigmui.Context) void {
    if (zigmui.widgets.begin_window(ctx, "Log Window", .{ .x = 350, .y = 40, .w = 300, .h = 200 }, .NONE)) {

        // output text panel
        ctx.layout.stack.back().row(&.{-1}, -25);
        zigmui.widgets.begin_panel(ctx, "Log Output", .{});
        const panel = ctx.container.current_container();

        ctx.layout.stack.back().row(&.{-1}, -1);
        zigmui.widgets.textarea(ctx, logbuf[0..logbuf_pos]);
        zigmui.widgets.end_panel(ctx);
        if (logbuf_updated) {
            // scroll to end
            panel.scroll.y = panel.content_size.y;
            logbuf_updated = false;
        }

        // input textbox + submit button
        {
            const S = struct {
                var buf: [128]u8 = undefined;

                fn slice() []const u8 {
                    var len: usize = buf.len;
                    for (buf) |ch, i| {
                        if (ch == 0) {
                            len = i;
                            break;
                        }
                    }
                    if (len > 0) {
                        std.log.debug("{}", .{len});
                    }
                    return buf[0..len];
                }
            };
            var submitted = false;
            ctx.layout.stack.back().row(&.{ -70, -1 }, 0);
            if (zigmui.widgets.textbox(ctx, &S.buf, .{}).has(.SUBMIT)) {
                ctx.input.set_focus(ctx.hash.last, .ARROW);
                submitted = true;
            }
            if (zigmui.widgets.button(ctx, .{ .text = "Submit" }, .{})) {
                submitted = true;
            }
            if (submitted) {
                write_log(S.slice());
                S.buf[0] = 0;
            }
        }

        zigmui.widgets.end_window(ctx);
    }
}

fn test_window(ctx: *zigmui.Context, bg: [*]f32) void {
    if (zigmui.widgets.begin_window(ctx, "Demo Window", .{ .x = 40, .y = 40, .w = 300, .h = 450 }, .NONE)) {
        const win = ctx.container.current_container();
        win.rect.w = std.math.max(win.rect.w, 240);
        win.rect.h = std.math.max(win.rect.h, 300);

        // window info
        if (zigmui.widgets.header(ctx, "Window Info", .{})) {
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
        if (zigmui.widgets.header(ctx, "Test Buttons", .{ .opt = .EXPANDED })) {
            ctx.layout.stack.back().row(&.{ 86, -110, -1 }, 0);
            zigmui.widgets.label(ctx, "Test buttons 1:");
            if (zigmui.widgets.button(ctx, .{ .text = "Button 1" }, .{})) {
                write_log("Pressed button 1");
            }
            if (zigmui.widgets.button(ctx, .{ .text = "Button 2" }, .{})) {
                write_log("Pressed button 2");
            }
            zigmui.widgets.label(ctx, "Test buttons 2:");
            if (zigmui.widgets.button(ctx, .{ .text = "Button 3" }, .{})) {
                write_log("Pressed button 3");
            }
            if (zigmui.widgets.button(ctx, .{ .text = "Popup" }, .{})) {
                zigmui.widgets.open_popup(ctx, "Test Popup");
            }
            if (zigmui.widgets.begin_popup(ctx, "Test Popup")) {
                _ = zigmui.widgets.button(ctx, .{ .text = "Hello" }, .{});
                _ = zigmui.widgets.button(ctx, .{ .text = "World" }, .{});
                zigmui.widgets.end_popup(ctx);
            }
        }

        // tree
        if (zigmui.widgets.header(ctx, "Tree and Text", .{ .opt = .EXPANDED })) {
            ctx.layout.stack.back().row(&.{ 140, -1 }, 0);
            const style = &ctx.command_drawer.style;
            {
                var layout = ctx.layout.begin_column(style);
                if (zigmui.widgets.begin_treenode(ctx, "Test 1", .{})) {
                    if (zigmui.widgets.begin_treenode(ctx, "Test 1a", .{})) {
                        zigmui.widgets.label(ctx, "Hello");
                        zigmui.widgets.label(ctx, "world");
                        zigmui.widgets.end_treenode(ctx);
                    }
                    if (zigmui.widgets.begin_treenode(ctx, "Test 1b", .{})) {
                        if (zigmui.widgets.button(ctx, .{ .text = "Button 1" }, .{})) {
                            write_log("Pressed button 1");
                        }
                        if (zigmui.widgets.button(ctx, .{ .text = "Button 2" }, .{})) {
                            write_log("Pressed button 2");
                        }
                        zigmui.widgets.end_treenode(ctx);
                    }
                    zigmui.widgets.end_treenode(ctx);
                }

                if (zigmui.widgets.begin_treenode(ctx, "Test 2", .{})) {
                    layout.row(&.{ 54, 54 }, 0);
                    if (zigmui.widgets.button(ctx, .{ .text = "Button 3" }, .{})) {
                        write_log("Pressed button 3");
                    }
                    if (zigmui.widgets.button(ctx, .{ .text = "Button 4" }, .{})) {
                        write_log("Pressed button 4");
                    }
                    if (zigmui.widgets.button(ctx, .{ .text = "Button 5" }, .{})) {
                        write_log("Pressed button 5");
                    }
                    if (zigmui.widgets.button(ctx, .{ .text = "Button 6" }, .{})) {
                        write_log("Pressed button 6");
                    }
                    zigmui.widgets.end_treenode(ctx);
                }

                if (zigmui.widgets.begin_treenode(ctx, "Test 3", .{})) {
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
        if (zigmui.widgets.header(ctx, "Background Color", .{ .opt = .EXPANDED })) {
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

var command_groups: [1]zigmui.CommandRange = undefined;
var command_buffer: [1024]u8 = undefined;

pub fn process_frame(ctx: *zigmui.Context, bg: [*]f32, frame: *zigmui.RenderFrame) !void {
    ctx.begin();

    style_window(ctx);
    log_window(ctx);
    test_window(ctx, bg);

    try ctx.end(frame);
}
