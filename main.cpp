#include <GL/glew.h>
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
  printf("[GLFW][code=%d] %s\n", code, description);
  fflush(stdout);
}

void myGlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action,
                       int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void GLAPIENTRY myGlDebugCallback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar *message,
                                  const void *userParam) {
  printf("[GL][source=0x%X; type=0x%X; id=0x%X; severity=0x%X] %s\n", source,
         type, id, severity, message);
}

int main() {
  if (!glfwInit())
    return __LINE__;
  InvokeOnDestroy _glfwTerminate(glfwTerminate);
  glfwSetErrorCallback(myGlfwErrorCallback);

  GLFWwindow *window =
      glfwCreateWindow(800, 600, "OpenGL Tutorial", nullptr, nullptr);
  glfwSetKeyCallback(window, myGlfwKeyCallback);

  glfwMakeContextCurrent(window);
  if (glewInit() != GLEW_OK)
    return __LINE__;

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(myGlDebugCallback, nullptr);

  glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
  }

  return 0;
}
