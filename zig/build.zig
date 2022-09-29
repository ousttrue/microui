const std = @import("std");
const c_pkg = std.build.Pkg{ .name = "c", .source = .{ .path = "c.zig" } };
const GLFW_BASE = "../_external/glfw";

const GLFW_SOURCES = .{
    GLFW_BASE ++ "/src/context.c",
    GLFW_BASE ++ "/src/init.c",
    GLFW_BASE ++ "/src/input.c",
    GLFW_BASE ++ "/src/monitor.c",
    GLFW_BASE ++ "/src/vulkan.c",
    GLFW_BASE ++ "/src/window.c",
    //
    GLFW_BASE ++ "/src/win32_init.c",
    GLFW_BASE ++ "/src/win32_joystick.c",
    GLFW_BASE ++ "/src/win32_monitor.c",
    GLFW_BASE ++ "/src/win32_time.c",
    GLFW_BASE ++ "/src/win32_thread.c",
    GLFW_BASE ++ "/src/win32_window.c",
    GLFW_BASE ++ "/src/wgl_context.c",
    GLFW_BASE ++ "/src/egl_context.c",
    GLFW_BASE ++ "/src/osmesa_context.c",
};
const GLFW_FLAGS = .{
    "-std=c99",
    "-D_GLFW_WIN32",
    "-DUNICODE",
    "-D_UNICODE",
};

fn build_static_glfw(exe: *std.build.LibExeObjStep) void {
    exe.addIncludePath(GLFW_BASE ++ "/include");
    exe.addCSourceFiles(&GLFW_SOURCES, &GLFW_FLAGS);
    exe.linkSystemLibrary("gdi32");
}

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
    build_static_glfw(exe);
    exe.linkLibC();
    exe.linkLibCpp();

    // link zig_renderer
    exe.addLibraryPath("../zig_renderer/zig-out/lib");
    exe.linkSystemLibrary("zig_renderer");

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
