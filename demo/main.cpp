#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "ui.h"
#include <microui.h>
#include <mu_context.h>
#include <renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MU_MOUSE button_map[256] = {
    MU_MOUSE_LEFT,
    MU_MOUSE_RIGHT,
    MU_MOUSE_MIDDLE,
};

// static const char key_map[256] = {
//   [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
//   [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
//   [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
//   [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
//   [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
//   [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
//   [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
//   [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
// };

static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) {
    len = strlen(text);
  }
  return r_get_text_width(text, len);
}

static int text_height(mu_Font font) { return r_get_text_height(); }

int mouse_x = 0;
int mouse_y = 0;
static void cursor_position_callback(GLFWwindow *window, double xpos,
                                     double ypos) {
  auto ctx = (mu_Context *)glfwGetWindowUserPointer(window);
  ctx->_input.mousemove(xpos, ypos);
  mouse_x = xpos;
  mouse_y = ypos;
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  auto b = button_map[button];
  auto ctx = (mu_Context *)glfwGetWindowUserPointer(window);
  if (action == GLFW_PRESS) {
    ctx->_input.mousedown(b);
  } else if (action == GLFW_RELEASE) {
    ctx->_input.mouseup(b);
  }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  auto ctx = (mu_Context *)glfwGetWindowUserPointer(window);
  ctx->_input.scroll(xoffset, yoffset * -30);
}

int main(int argc, char **argv) {
  if (!glfwInit())
    exit(1);

  /* Create a windowed mode window and its OpenGL context */
  GLFWwindow *window = glfwCreateWindow(1200, 1000, "Hello World", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  r_init();

  // init microui
  auto ctx = mu_new(text_width, text_height);
  glfwSetWindowUserPointer(window, ctx);

  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);

  float bg[3] = {90, 95, 100};

  /* main loop */
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    //   case SDL_TEXTINPUT:
    //     mu_input_text(ctx, e.text.text);
    //     break;

    //   case SDL_KEYDOWN:
    //   case SDL_KEYUP: {
    //     int c = key_map[e.key.keysym.sym & 0xff];
    //     if (c && e.type == SDL_KEYDOWN) {
    //       mu_input_keydown(ctx, c);
    //     }
    //     if (c && e.type == SDL_KEYUP) {
    //       mu_input_keyup(ctx, c);
    //     }
    //     break;
    //   }
    //   }
    // }
    UIRenderFrame command;
    process_frame(ctx, bg, &command);
    render(width, height, bg, &command);

    glfwSwapBuffers(window);
  }

  return 0;
}
