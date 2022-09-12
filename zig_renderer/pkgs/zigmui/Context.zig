const std = @import("std");
const Rect = @import("./Rect.zig");
const Vec2 = @import("./Vec2.zig");
const Color32 = @import("./Color32.zig");
const Hash = @import("./Hash.zig");
const Container = @import("./Container.zig");
const ContainerManager = @import("./ContainerManager.zig");
const CommandDrawer = @import("./CommandDrawer.zig");
const RenderFrame = @import("./RenderFrame.zig");
const Input = @import("./Input.zig");
const LayoutManager = @import("./LayoutManager.zig");
const Layout = @import("./Layout.zig");
const Tree = @import("./Tree.zig");
const TextEditor = @import("./TextEditor.zig");
const ROOTLIST_SIZE = 32;
const COMMANDLIST_SIZE = 256 * 1024;

const Self = @This();

frame: u32 = 0,
hash: Hash = .{},
container: ContainerManager = .{},
command_drawer: CommandDrawer = .{},
input: Input = .{},
layout: LayoutManager = .{},
tree: Tree = .{},
editor: TextEditor = .{},

command_groups: [ROOTLIST_SIZE]RenderFrame.CommandRange = undefined,
command_buffer: [COMMANDLIST_SIZE]u8 = undefined,

pub fn mouse_over(self: Self, rect: Rect) bool {
    if (!rect.overlaps_vec2(self.input.mouse_pos)) {
        return false;
    }
    if (!self.command_drawer.clip_stack.overlaps(self.input.mouse_pos)) {
        return false;
    }
    if (!self.container.in_hover_root()) {
        return false;
    }
    return true;
}

pub fn begin(self: *Self) void {
    self.command_drawer.begin();
    self.container.begin();
    self.input.begin();
    self.frame += 1;
}

pub fn end(self: *Self, command: *RenderFrame) !void {
    self.hash.end();
    self.layout.end();
    const mouse_pressed = self.input.end();
    self.container.end(mouse_pressed, command);
    self.command_drawer.end(command);
    command.cursor_shape = if (self.input.focus) |focus|
        focus.shape
    else if (self.input.hover) |hover|
        hover.shape
    else
        .ARROW;
}
