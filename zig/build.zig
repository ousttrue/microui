const std = @import("std");
const c_pkg = std.build.Pkg{ .name = "c", .source = .{ .path = "c.zig" } };
const GLFW_BASE = "../cpp/_external/glfw";
const CPP_BUILD_BASE = "../build";
const CPP_BASE = "../cpp";

const zigmui_pkg = std.build.Pkg{
    .name = "zigmui",
    .source = .{ .path = "pkgs/zigmui/main.zig" },
    .dependencies = &.{c_pkg},
};

pub fn build(b: *std.build.Builder) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("microui", "src/main.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.addPackage(c_pkg);
    exe.addPackage(zigmui_pkg);
    exe.addIncludePath(GLFW_BASE ++ "/include");
    exe.addIncludePath(CPP_BASE ++ "/microui");
    exe.addIncludePath(CPP_BASE ++ "/uirf");
    exe.addIncludePath(CPP_BASE ++ "/gl_renderer");
    // exe.addCSourceFiles(&.{
    //     "src/microui.cpp",
    //     "src/renderer.cpp",
    //     "src/atlas.c",
    // }, &.{
    //     "-DBUILD_MICROUI",
    // });

    exe.linkLibC();
    exe.linkLibCpp();

    exe.addLibraryPath("../cpp/build/Debug/lib");
    exe.linkSystemLibrary("microui");
    exe.linkSystemLibrary("gl_renderer");
    exe.linkSystemLibrary("glfw3dll");
    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    const exe_tests = b.addTest("src/main.zig");
    exe_tests.setTarget(target);
    exe_tests.setBuildMode(mode);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&exe_tests.step);
}
