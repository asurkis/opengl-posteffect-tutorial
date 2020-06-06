#include <GLFW/glfw3.h>
#include <cstdio>
#include <functional>

class InvokeOnDestroy {
  std::function<void()> f;

public:
  InvokeOnDestroy(std::function<void()> &&fn) : f(fn) {}
  ~InvokeOnDestroy() { f(); }
};

void myGlfwErrorCallback(int code, const char *description) {
  printf("[GLFW] %d: %s\n", code, description);
  fflush(stdout);
}

void myGlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action,
                       int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main() {
  if (!glfwInit())
    return __LINE__;
  InvokeOnDestroy _glfwTerminate(glfwTerminate);
  glfwSetErrorCallback(myGlfwErrorCallback);

  GLFWwindow *window =
      glfwCreateWindow(800, 600, "OpenGL Tutorial", nullptr, nullptr);
  glfwSetKeyCallback(window, myGlfwKeyCallback);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  return 0;
}
