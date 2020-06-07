#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstdio>
#include <exception>
#include <functional>
#include <glm/glm.hpp>
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
DEFINE_GL_ARRAY_HELPER(Textures, glGenTextures, glDeleteTextures)
DEFINE_GL_ARRAY_HELPER(Framebuffers, glGenFramebuffers, glDeleteFramebuffers)

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
      glfwCreateWindow(1200, 630, "OpenGL Tutorial", nullptr, nullptr);
  glfwSetKeyCallback(window, myGlfwKeyCallback);

  glfwMakeContextCurrent(window);
  if (glewInit() != GLEW_OK)
    return __LINE__;

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(myGlDebugCallback, nullptr);

  ShaderProgram mainProgram, postProgram;
  mainProgram.init("s1.vert", "s1.frag");
  postProgram.init("s2.vert", "s2.frag");

  Buffers buffers(3);
  VertexArrays vertexArrays(2);
  Textures textures(2);
  Framebuffers framebuffers(1);
  GLint attribLocation;

  glBindVertexArray(vertexArrays[1]);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
  {
    GLfloat fillTriangle[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, //
        3.0f,  -1.0f, 2.0f, 0.0f, //
        -1.0f, 3.0f,  0.0f, 2.0f, //
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(fillTriangle), fillTriangle,
                 GL_STATIC_DRAW);
  }
  attribLocation = glGetAttribLocation(postProgram, "vertexPosition");
  glEnableVertexAttribArray(attribLocation);
  glVertexAttribPointer(attribLocation, 2, GL_FLOAT, GL_FALSE,
                        4 * sizeof(GLfloat), 0);
  attribLocation = glGetAttribLocation(postProgram, "vertexTextureCoords");
  glEnableVertexAttribArray(attribLocation);
  glVertexAttribPointer(attribLocation, 2, GL_FLOAT, GL_FALSE,
                        4 * sizeof(GLfloat), (GLvoid *)(2 * sizeof(GLfloat)));
  glBindVertexArray(vertexArrays[0]);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
  GLuint indexCount;
  {
    Assimp::Importer importer;
    const aiScene *scene =
        importer.ReadFile("scene.obj", aiProcess_Triangulate);
    const aiMesh *mesh = scene->mMeshes[0];
    glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * 3 * sizeof(GLfloat),
                 mesh->mVertices, GL_STATIC_DRAW);
    std::vector<GLuint> indices;
    for (int i = 0; i < mesh->mNumFaces; ++i)
      for (int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
        indices.push_back(mesh->mFaces[i].mIndices[j]);
    indexCount = indices.size();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint),
                 indices.data(), GL_STATIC_DRAW);
  }
  attribLocation = glGetAttribLocation(mainProgram, "vertexPosition");
  glEnableVertexAttribArray(attribLocation);
  glVertexAttribPointer(attribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  const int MAX_WIDTH = 4096;
  const int MAX_HEIGHT = 4096;
  glBindTexture(GL_TEXTURE_2D, textures[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, MAX_WIDTH, MAX_HEIGHT, 0, GL_RGB,
               GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, textures[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, MAX_WIDTH, MAX_HEIGHT, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         textures[0], 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         textures[1], 0);
  GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glBindVertexArray(vertexArrays[1]);
  glUseProgram(postProgram);
  glUniform1i(glGetUniformLocation(postProgram, "renderTexture"), 0);
  glUniform1i(glGetUniformLocation(postProgram, "depthTexture"), 1);
  glUniform2f(glGetUniformLocation(postProgram, "reverseMaxSize"),
              1.0f / MAX_WIDTH, 1.0f / MAX_HEIGHT);
  glUseProgram(0);
  glBindVertexArray(0);

  GLint ulMatModel = glGetUniformLocation(mainProgram, "matModel");
  GLint ulMatView = glGetUniformLocation(mainProgram, "matView");
  GLint ulMatProjection = glGetUniformLocation(mainProgram, "matProjection");
  GLint ulTextureScale = glGetUniformLocation(postProgram, "textureScale");
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.75f, 0.75f, 0.75f, 0.0f);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vertexArrays[0]);
    glUseProgram(mainProgram);

    float angle = 0.125f; //0.125f * glfwGetTime();
    float sin = glm::sin(angle);
    float cos = glm::cos(angle);
    glm::vec3 pos(2.0f * sin, 2.0f * cos, 0.125f);
    glm::vec3 forward = glm::normalize(-pos);
    glm::vec3 up(0.0f, 0.0f, 1.0f);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    up = glm::normalize(glm::cross(right, forward));
    float zNear = 0.0625f;
    float zFar = 32.0f;

    glm::mat4 matModel(1.0f, 0.0f, 0.0f, 0.0f, //
                       0.0f, 1.0f, 0.0f, 0.0f, //
                       0.0f, 0.0f, 1.0f, 0.0f, //
                       0.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 matView(right.x, up.x, forward.x, 0.0f, //
                      right.y, up.y, forward.y, 0.0f, //
                      right.z, up.z, forward.z, 0.0f, //
                      0.0f, 0.0f, 0.0f, 1.0f);
    matView *= glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, //
                         0.0f, 1.0f, 0.0f, 0.0f, //
                         0.0f, 0.0f, 1.0f, 0.0f, //
                         -pos.x, -pos.y, -pos.z, 1.0f);
    glm::mat4 matProjection(
        (float)framebufferHeight / framebufferWidth, 0.0f, 0.0f, 0.0f, //
        0.0f, 1.0f, 0.0f, 0.0f,                                        //
        0.0f, 0.0f, (zFar + zNear) / (zFar - zNear), 1.0f,             //
        0.0f, 0.0f, -2.0f * zFar * zNear / (zFar - zNear), 0.0f);

    glUniformMatrix4fv(ulMatModel, 1, GL_FALSE, &matModel[0][0]);
    glUniformMatrix4fv(ulMatView, 1, GL_FALSE, &matView[0][0]);
    glUniformMatrix4fv(ulMatProjection, 1, GL_FALSE, &matProjection[0][0]);

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glBindVertexArray(vertexArrays[1]);
    glUseProgram(postProgram);
    glUniform2f(ulTextureScale, (GLfloat)framebufferWidth / MAX_WIDTH,
                (GLfloat)framebufferHeight / MAX_HEIGHT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glfwSwapBuffers(window);
  }

  return 0;
}
