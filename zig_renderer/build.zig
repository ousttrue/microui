const std = @import("std");

const c_pkg = std.build.Pkg{ .name = "c", .source = .{ .path = "c.zig" } };
const zigmui_pkg = std.build.Pkg{
    .name = "zigmui",
    .source = .{ .path = "pkgs/zigmui/main.zig" },
    .dependencies = &.{c_pkg},
};


pub fn build(b: *std.build.Builder) void {
    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const lib = b.addSharedLibrary("zig_renderer", "src/main.zig", .unversioned);
    lib.setBuildMode(mode);
    lib.addPackage(c_pkg);
    lib.addPackage(zigmui_pkg);
    lib.linkLibC();
    lib.linkLibCpp();
    lib.linkSystemLibrary("OpenGL32");
    lib.addIncludePath("../cpp/uirf");
    lib.addIncludePath("../cpp/_external/glfw/deps");
    lib.addCSourceFile("../cpp/_external/glfw/deps/glad_gl.c", &.{});
    lib.install();

    const main_tests = b.addTest("src/main.zig");
    main_tests.setBuildMode(mode);

    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&main_tests.step);
}
