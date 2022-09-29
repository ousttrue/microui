const std = @import("std");

const zigmui_pkg = std.build.Pkg{
    .name = "zigmui",
    .source = .{ .path = "pkgs/zigmui/main.zig" },
};

pub fn build(b: *std.build.Builder) void {
    const lib = b.addSharedLibrary("zig_renderer", "src/main.zig", .unversioned);

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();
    lib.setBuildMode(mode);

    const target = b.standardTargetOptions(.{});
    lib.setTarget(target);

    lib.addPackage(zigmui_pkg);
    if (target.cpu_arch == std.Target.Cpu.Arch.wasm32) {
        lib.stack_size = 6 * 1024 * 1024;
    } else {
        lib.linkLibC();
        lib.linkLibCpp();
        lib.addIncludePath("../_external/glfw/deps");
        lib.addCSourceFile("../_external/glfw/deps/glad_gl.c", &.{});
        lib.addCSourceFile("src/glad_placeholders.c", &.{});
    }
    lib.install();

    const main_tests = b.addTest("src/main.zig");
    main_tests.setBuildMode(mode);

    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&main_tests.step);
}
