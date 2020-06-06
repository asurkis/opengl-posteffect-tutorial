#version 330 core

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;
in vec3 vertexPosition;

void main() {
  gl_Position = matProjection * matView * matModel * vec4(vertexPosition, 1);
}
