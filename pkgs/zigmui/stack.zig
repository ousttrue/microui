pub fn Stack(comptime T: type, comptime N: usize) type {
    return struct {
        const Self = @This();
        items: [N]T = undefined,
        idx: usize = 0,

        // const T *begin() const { return items; }
        // T *begin() { return items; }
        // T *end() { return items + idx; }
        pub fn size(self: Self) usize {
            return self.idx;
        }
        // const T &get(int i) const { return items[i]; }
        // T &get(int i) { return items[i]; }
        // T &back() {
        //     assert(idx > 0);
        //     return items[idx - 1];
        // }
        // void grow(int size) {
        //     assert(idx + size < N);
        //     idx += size;
        // }
        pub fn back(self: Self) T {
            return self.items[self.idx - 1];
        }
        // void push(const T &val) {
        //     assert(this->idx < (int)(sizeof(this->items) / sizeof(*this->items)));
        //     this->items[this->idx] = (val);
        //     this->idx++; /* incremented after incase `val` uses this value */
        // }

        // void pop() {
        //     assert(this->idx > 0);
        //     this->idx--;
        // }

        // void clear() { idx = 0; }
    };
}
