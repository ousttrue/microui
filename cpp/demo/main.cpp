#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "ui.h"
#include <microui.h>
#include <mu_context.h>
#include <renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Windows.h>

static MU_MOUSE button_map[256] = {
    MU_MOUSE_LEFT,
    MU_MOUSE_RIGHT,
    MU_MOUSE_MIDDLE,
};

static MU_KEY key_map[1024] = {};

static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) {
    len = strlen(text);
  }
  return MUI_RENDERER_get_text_width(text, len);
}

static int text_height(mu_Font font) { return MUI_RENDERER_get_text_height(); }

int mouse_x = 0;
int mouse_y = 0;
static void cursor_position_callback(GLFWwindow *window, double xpos,
                                     double ypos) {
  auto ctx = (mu_Context *)glfwGetWindowUserPointer(window);
  ctx->_input.set_mousemove(xpos, ypos);
  mouse_x = xpos;
  mouse_y = ypos;
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  auto b = button_map[button];
  auto ctx = (mu_Context *)glfwGetWindowUserPointer(window);
  if (action == GLFW_PRESS) {
    ctx->_input.set_mousedown(b);
  } else if (action == GLFW_RELEASE) {
    ctx->_input.set_mouseup(b);
  }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  auto ctx = (mu_Context *)glfwGetWindowUserPointer(window);
  ctx->_input.add_scroll_delta(xoffset, yoffset * -30);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  auto ctx = (mu_Context *)glfwGetWindowUserPointer(window);
  auto c = key_map[key];
  switch (action) {
  case GLFW_PRESS:
    ctx->_input.set_keydown(c);
    break;

  case GLFW_RELEASE:
    ctx->_input.set_keyup(c);
    break;

  default:
    break;
  }
}

void character_callback(GLFWwindow *window, unsigned int codepoint) {

  wchar_t wstr[] = {(wchar_t)codepoint, 0};
  int sizeNeeded =
      WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
  char encodedStr[4];
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, encodedStr, sizeNeeded, NULL, NULL);
  auto ctx = (mu_Context *)glfwGetWindowUserPointer(window);
  if (encodedStr[sizeNeeded - 1] == 0) {
    // remove zero
    sizeNeeded -= 1;
  }
  ctx->_input.push_utf8(encodedStr, sizeNeeded);
}

int main(int argc, char **argv) {
  if (!glfwInit()) {
    exit(1);
  }

  // Create a windowed mode window and its OpenGL context
  auto window = glfwCreateWindow(1200, 1000, "Hello World", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);

  MUI_RENDERER_init(glfwGetProcAddress);

  // init microui
  auto ctx = mu_new(text_width, text_height);
  glfwSetWindowUserPointer(window, ctx);

  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetKeyCallback(window, key_callback);
  key_map[GLFW_KEY_LEFT_SHIFT] = MU_KEY_SHIFT;
  key_map[GLFW_KEY_RIGHT_SHIFT] = MU_KEY_SHIFT;
  key_map[GLFW_KEY_LEFT_CONTROL] = MU_KEY_CTRL;
  key_map[GLFW_KEY_RIGHT_CONTROL] = MU_KEY_CTRL;
  key_map[GLFW_KEY_LEFT_ALT] = MU_KEY_ALT;
  key_map[GLFW_KEY_RIGHT_ALT] = MU_KEY_ALT;
  key_map[GLFW_KEY_ENTER] = MU_KEY_RETURN;
  key_map[GLFW_KEY_BACKSPACE] = MU_KEY_BACKSPACE;
  glfwSetCharCallback(window, character_callback);

  float bg[3] = {90, 95, 100};

  // main loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    UIRenderFrame command;
    process_frame(ctx, bg, &command);
    MUI_RENDERER_render(width, height, bg, &command);

    glfwSwapBuffers(window);
  }

  return 0;
}
