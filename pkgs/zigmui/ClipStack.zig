const std = @import("std");
const Rect = @import("./Rect.zig");
const Stack = @import("./stack.zig").Stack;
const CLIP = enum { NONE, PART, ALL };
pub const UNCLIPPED_RECT = Rect{ .x = 0, .y = 0, .w = 0x1000000, .h = 0x1000000 };

const CLIPSTACK_SIZE = 32;
const Self = @This();

clip_stack: Stack(Rect, CLIPSTACK_SIZE) = .{},

pub fn end(self: Self) void {
    std.debug.assert(self.clip_stack.size() == 0);
}

pub fn intersect(self: Self, rect: Rect) Rect {
    return rect.intersect(self.clip_stack.back());
}

pub fn push_unclipped_rect(self: *Self) void {
    self.clip_stack.push(UNCLIPPED_RECT);
}

//   void push(UIRect rect) {
//     UIRect last = this->clip_stack.back();
//     this->clip_stack.push(rect.intersect(last));
//   }
//   UIRect back() const { return clip_stack.back(); }
pub fn pop(self: *Self) void {
    self.clip_stack.pop();
}

//   MU_CLIP check_clip(UIRect r) {
//     UIRect cr = this->clip_stack.back();
//     if (r.x > cr.x + cr.w || r.x + r.w < cr.x || r.y > cr.y + cr.h ||
//         r.y + r.h < cr.y) {
//       return MU_CLIP::ALL;
//     }
//     if (r.x >= cr.x && r.x + r.w <= cr.x + cr.w && r.y >= cr.y &&
//         r.y + r.h <= cr.y + cr.h) {
//       return MU_CLIP::NONE;
//     }
//     return MU_CLIP::PART;
//   }
// };
