pub fn strlen(src: *const u8) usize {
    var i: usize = 0;
    var p = @ptrCast([*]const u8, src);
    while (p[0] != 0) : (i += 1) {}
    return i;
}
