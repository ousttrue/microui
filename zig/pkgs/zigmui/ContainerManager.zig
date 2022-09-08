const std = @import("std");
const c = @import("c");
const Pool = @import("./pool.zig").Pool;
const Hash = @import("./Hash.zig");
const Vec2 = @import("./Vec2.zig");
const Container = @import("./Container.zig");
const Stack = @import("./stack.zig").Stack;
const Input = @import("./Input.zig");

const CONTAINERSTACK_SIZE = 32;
const CONTAINERPOOL_SIZE = 48;
const ROOTLIST_SIZE = 32;

const Self = @This();

// container pool managment
container_pool: Pool(CONTAINERPOOL_SIZE) = .{},
containers: [CONTAINERPOOL_SIZE]Container = undefined,

// container stack
container_stack: Stack(*Container, CONTAINERSTACK_SIZE) = .{},

// root window management
last_zindex: i32 = 0,
hover_root: ?*const Container = null,
next_hover_root: ?*Container = null,
root_list: Stack(*Container, ROOTLIST_SIZE) = .{},
root_window_ranges: [ROOTLIST_SIZE]c.struct_UICommandRange = undefined,

//   static int compare_zindex(const void *a, const void *b) {
//     return (*(mu_Container **)a).zindex - (*(mu_Container **)b).zindex;
//   }

pub fn begin(self: *Self) void {
    self.hover_root = self.next_hover_root;
    self.next_hover_root = null;
    self.root_list.clear();
}

fn compare_zindex(a: ?*const anyopaque, b: ?*const anyopaque) callconv(.C) c_int {
    const A = @ptrCast(*const *Container, @alignCast(@alignOf(*Container), a));
    const B = @ptrCast(*const *Container, @alignCast(@alignOf(*Container), b));
    return A.*.zindex - B.*.zindex;
}

pub fn end(self: *Self, mouse_pressed: Input.MOUSE_BUTTON, command: *c.struct_UIRenderFrame) void {
    std.debug.assert(self.container_stack.size() == 0);

    // bring hover root to front if mouse was pressed
    if (mouse_pressed != .NONE) {
        if (self.next_hover_root) |next_hover_root| {
            // self.next_hover_root.zindex < self.last_zindex &&
            if (next_hover_root.zindex >= 0) {
                self.bring_to_front(next_hover_root);
            }
        }
    }

    // sort root containers by zindex
    const n = self.root_list.size();
    c.qsort(@ptrCast(*anyopaque, self.root_list.begin()), n, @sizeOf(*Container), compare_zindex);

    // const root_end = self.root_list.end();
    // var p = @ptrCast([*]c.struct_UICommandRange, &self.root_window_ranges[0]);
    // var it = self.root_list.begin();
    // while (it != root_end) : ({
    //     it += 1;
    //     p += 1;
    // }) {
    //     p.*.head = @intCast(u32, it.*.head);
    //     p.*.tail = @intCast(u32, it.*.tail);
    // }
    for (self.root_list.slice()) |root, i| {
        self.root_window_ranges[i].head = @intCast(u32, root.head);
        self.root_window_ranges[i].tail = @intCast(u32, root.tail);
    }

    command.command_groups = &self.root_window_ranges[0];
    command.command_group_count = @intCast(u32, self.root_list.size());
}

pub fn bring_to_front(self: *Self, cnt: *Container) void {
    self.last_zindex += 1;
    cnt.zindex = self.last_zindex;
}

pub fn is_hover_root(self: Self, cnt: *Container) bool {
    return self.hover_root == cnt;
}

/// try to get existing container from pool
pub fn get_container(self: *Self, id: Hash.Id, opt: Input.OPT, frame: u32) ?*Container {
    if (self.container_pool.get_index(id)) |idx| {
        if (self.containers[idx].open or !opt.has(.CLOSED)) {
            self.container_pool.update(frame, idx);
        }
        return &self.containers[idx];
    }

    if (opt.has(.CLOSED)) {
        return null;
    }

    // container not found in pool: init new container
    const idx = self.container_pool.init(frame, id);
    const cnt = &self.containers[idx];
    cnt.reset();
    self.bring_to_front(cnt);
    return cnt;
}

//   void push(mu_Container *cnt)
//   {
//     _container_stack.push(cnt);
//   }

pub fn current_container(self: *Self) *Container {
    return self.container_stack.back().*;
}

pub fn pop(self: *Self) void {
    // pop container, layout and id
    self.container_stack.pop();
}

pub fn in_hover_root(self: Self) bool {
    if (self.hover_root) |hover_root| {
        const slice = self.container_stack.slice_const();
        var i = @intCast(i32, slice.len - 1);
        while (i >= 0) : (i -= 1) {
            const cnt = slice[@intCast(usize, i)];
            if (cnt == hover_root) {
                return true;
            }
            // only root containers have their `head` field set; stop searching if
            // we've reached the current root container
            if (cnt.head > 0) {
                break;
            }
        }
    }
    return false;
}

pub fn begin_root_container(self: *Self, cnt: *Container, command_head: usize, mouse_pos: Vec2) void {
    self.container_stack.push(cnt);
    // push container to roots list and push head command
    self.root_list.push(cnt);
    cnt.head = command_head;
    // set as hover root if the mouse is overlapping this container and it has a
    // higher zindex than the current hover root
    if (cnt.rect.overlaps_vec2(mouse_pos)) {
        if (self.next_hover_root) |next_hover_root| {
            if (cnt.zindex > next_hover_root.zindex) {
                self.next_hover_root = cnt;
            }
        } else {
            self.next_hover_root = cnt;
        }
    }
}

pub fn open_popup(self: *Self, id: Hash.Id, mouse_pos: Vec2, frame: u32) void {
    var cnt = self.get_container(id, .NONE, frame) orelse unreachable;
    // set as hover root so popup isn't closed in begin_window_ex()
    self.next_hover_root = cnt;
    self.hover_root = cnt;
    // position at mouse cursor, open and bring-to-front
    cnt.rect = .{ .x = mouse_pos.x, .y = mouse_pos.y, .w = 1, .h = 1 };
    cnt.open = true;
    self.bring_to_front(cnt);
}
