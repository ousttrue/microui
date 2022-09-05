const Pool = @import("./pool.zig").Pool;
const Hash = @import("./Hash.zig");
const TREENODEPOOL_SIZE = 48;
const Self = @This();

treenode_pool: Pool(TREENODEPOOL_SIZE) = .{},

pub fn get(self: Self, id: Hash.Id) ?usize {
    return self.treenode_pool.get_index(id);
}

pub fn update(self: *Self, id: Hash.Id, idx: usize, active: bool, frame: u32) void {
    if (idx >= 0) {
        if (active) {
            self.treenode_pool.update(frame, idx);
        } else {
            self.treenode_pool.clear(idx);
        }
    } else if (active) {
        self.treenode_pool.init(frame, id);
    }
}
