pub usingnamespace @cImport({
    @cDefine("GLFW_INCLUDE_NONE", "1");
    @cInclude("GLFW/glfw3.h");    
    @cInclude("microui.h");
    @cInclude("renderer.h");

    @cInclude("string.h");
    @cInclude("stdlib.h");
    @cInclude("stdio.h");
});
