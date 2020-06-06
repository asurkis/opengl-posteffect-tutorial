#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <exception>
#include <functional>
#include <string>
#include <vector>

class InvokeOnDestroy {
  std::function<void()> f;

public:
  InvokeOnDestroy(std::function<void()> &&fn) : f(fn) {}
  ~InvokeOnDestroy() { f(); }
};

class Shader {
  GLuint id;

  void load(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f)
      throw std::string("Could not open file ") + filename;
    InvokeOnDestroy _fclose([&]() { fclose(f); });

    std::string src;
    int c;
    while ((c = getc(f)) != EOF)
      src.push_back(c);

    const GLchar *string = src.data();
    const GLint length = src.length();
    glShaderSource(id, 1, &string, &length);
  }

  void compile() {
    glCompileShader(id);
    GLint status;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    if (!status) {
      GLchar infoLog[2048];
      GLsizei length;
      glGetShaderInfoLog(id, sizeof(infoLog) / sizeof(infoLog[0]), &length,
                         infoLog);
      throw std::string(infoLog);
    }
  }

public:
  Shader(GLenum type) : id(glCreateShader(type)) {}
  ~Shader() { glDeleteShader(id); }
  operator GLuint() { return id; }

  void init(const char *filename) {
    load(filename);
    compile();
  }
};

class ShaderProgram {
  GLuint id;

  void link() {
    glLinkProgram(id);
    GLint status;
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    if (!status) {
      GLchar infoLog[2048];
      GLsizei length;
      glGetProgramInfoLog(id, sizeof(infoLog) / sizeof(infoLog[0]), &length,
                          infoLog);
      fputs(infoLog, stderr);
      throw std::exception();
    }
  }

  void validate() {
    glValidateProgram(id);
    GLint status;
    glGetProgramiv(id, GL_VALIDATE_STATUS, &status);
    if (!status) {
      GLchar infoLog[2048];
      GLsizei length;
      glGetProgramInfoLog(id, sizeof(infoLog) / sizeof(infoLog[0]), &length,
                          infoLog);
      fputs(infoLog, stderr);
      fflush(stderr);
      throw std::exception();
    }
  }

public:
  ShaderProgram() : id(glCreateProgram()) {}
  ~ShaderProgram() { glDeleteProgram(id); }
  operator GLuint() { return id; }

  void init(const char *vertSrc, const char *fragSrc) {
    Shader vert(GL_VERTEX_SHADER);
    Shader frag(GL_FRAGMENT_SHADER);
    vert.init(vertSrc);
    frag.init(fragSrc);
    glAttachShader(id, vert);
    glAttachShader(id, frag);
    link();
    validate();
  }
};

#define DEFINE_GL_ARRAY_HELPER(name, gen, del)                                 \
  struct name : public std::vector<GLuint> {                                   \
    name(size_t n) : std::vector<GLuint>(n) { gen(n, data()); }                \
    ~name() { del(size(), data()); }                                           \
  };
DEFINE_GL_ARRAY_HELPER(Buffers, glGenBuffers, glDeleteBuffers)
DEFINE_GL_ARRAY_HELPER(VertexArrays, glGenVertexArrays, glDeleteVertexArrays)

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
  fflush(stdout);
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

  ShaderProgram mainProgram, postProgram;
  mainProgram.init("s1.vert", "s1.frag");

  Buffers buffers(1);
  VertexArrays vertexArrays(1);
  GLint attribLocation;

  glBindVertexArray(vertexArrays[0]);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
  {
    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f, //
        -0.5f, 0.5f,  0.0f, //
        0.5f,  0.0f,  0.0f, //
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  }
  attribLocation = glGetAttribLocation(mainProgram, "vertexPosition");
  glEnableVertexAttribArray(attribLocation);
  glVertexAttribPointer(attribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(vertexArrays[0]);
    glUseProgram(mainProgram);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // glUseProgram(0);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
  }

  return 0;
}
