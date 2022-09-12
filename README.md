# zigmui ZIG Micro UI

[original](./README.md)

```
+-------+ mouse/keyboard input
|microui|<----------+
+-------+           |
 |build commands    |
 v                  |
+---------------+  +-----------+
|UIRenderFrame.h|  |main       |
+---------------+  |glfw window|
 |render commands  +-----------+
 v                  |
+-----------+       |
|gl_renderer|<------+ main loop
|font atlas |render
+-----------+
```

* [x] zig renderer
* [x] OpenGL4 vbo
* [x] wasm version

## zig_renderer

It can be compiled to both Desktop Native and Wasm.

## wasm build

<https://ousttrue.github.io/microui/>
