const Hash = @import("./Hash.zig");

const Item = struct {
    const Self = @This();
    id: Hash.Id = 0,
    last_update_frame: u32 = 0,

    fn clear(self: *Self) void {
        self.id = 0;
        self.last_update_frame = 0;
    }
};

pub fn Pool(comptime N: usize) type {
    return struct {
        const Self = @This();

        items: [N]Item = undefined,

        pub fn clear(self: *Self, idx: usize) void {
            self.items[idx].clear();
        }

        pub fn init(self: *Self, frame: u32, id: Hash.Id) usize {
            var n: ?usize = null;
            var f = frame;
            for (self.items) |item, i| {
                if (item.last_update_frame < f) {
                    f = item.last_update_frame;
                    n = i;
                }
            }
            const idx = n orelse {
                @panic("no item");
            };
            self.items[idx].id = id;
            self.update(frame, idx);
            return idx;
        }

        pub fn get_index(self: Self, id: Hash.Id) ?usize {
            for (self.items) |item, i| {
                if (item.id == id) {
                    return i;
                }
            }
            return null;
        }

        pub fn update(self: *Self, frame: u32, idx: usize) void {
            self.items[idx].last_update_frame = frame;
        }
    };
}
