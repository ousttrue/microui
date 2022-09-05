const std = @import("std");
const Rect = @import("./Rect.zig");
const Stack = @import("./stack.zig").Stack;
const Layout = @import("./Layout.zig");

const LAYOUTSTACK_SIZE = 16;

const Self = @This();

layout_stack: Stack(Layout, LAYOUTSTACK_SIZE) = .{},
last_rect: Rect = .{},

pub fn end(self: Self) void {
    std.debug.assert(self.layout_stack.size() == 0);
}

//   void begin_column(const mu_Style *style) {
//     _layout_stack.push(mu_Layout(next(style), UIVec2(0, 0)));
//   }

//   void end_column() {
//     auto b = &_layout_stack.back();
//     _layout_stack.pop();
//     // inherit position/next_row/max from child layout if they are greater
//     auto a = &_layout_stack.back();
//     a->position.x =
//         mu_max(a->position.x, b->position.x + b->body.x - a->body.x);
//     a->next_row = mu_max(a->next_row, b->next_row + b->body.y - a->body.y);
//     a->max.x = mu_max(a->max.x, b->max.x);
//     a->max.y = mu_max(a->max.y, b->max.y);
//   }

//   void set_width(int width) { _layout_stack.back().size.x = width; }

//   void set_height(int height) { _layout_stack.back().size.y = height; }

//   void set_next(UIRect r, bool relative) {
//     mu_Layout *layout = &_layout_stack.back();
//     layout->next = r;
//     layout->next_type = relative ? LAYOUT_RELATIVE : LAYOUT_ABSOLUTE;
//   }

//   UIRect next(const mu_Style *style) {
//     // auto style = _command_drawer.style();
//     mu_Layout *layout = &_layout_stack.back();

//     UIRect res;
//     if (layout->next_type) {
//       // handle rect set by `mu_layout_set_next`
//       int type = layout->next_type;
//       layout->next_type = LAYOUT_NONE;
//       res = layout->next;
//       if (type == LAYOUT_ABSOLUTE) {
//         return (_last_rect = res);
//       }
//     } else {
//       // handle next row
//       if (layout->item_index == layout->items) {
//         _layout_stack.back().row(layout->items, nullptr, layout->size.y);
//       }

//       // position
//       res.x = layout->position.x;
//       res.y = layout->position.y;

//       // size
//       res.w = layout->items > 0 ? layout->widths[layout->item_index]
//                                 : layout->size.x;
//       res.h = layout->size.y;
//       if (res.w == 0) {
//         res.w = style->size.x + style->padding * 2;
//       }
//       if (res.h == 0) {
//         res.h = style->size.y + style->padding * 2;
//       }
//       if (res.w < 0) {
//         res.w += layout->body.w - res.x + 1;
//       }
//       if (res.h < 0) {
//         res.h += layout->body.h - res.y + 1;
//       }

//       layout->item_index++;
//     }

//     // update position
//     layout->position.x += res.w + style->spacing;
//     layout->next_row = mu_max(layout->next_row, res.y + res.h + style->spacing);

//     // apply body offset
//     res.x += layout->body.x;
//     res.y += layout->body.y;

//     // update max position
//     layout->max.x = mu_max(layout->max.x, res.x + res.w);
//     layout->max.y = mu_max(layout->max.y, res.y + res.h);

//     return (_last_rect = res);
//   }
