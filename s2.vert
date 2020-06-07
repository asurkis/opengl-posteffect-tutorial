#version 330 core

uniform vec2 textureScale;
in vec2 vertexPosition;
in vec2 vertexTextureCoords;
out vec2 textureCoords;

void main() {
  gl_Position = vec4(vertexPosition, 0, 1);
  textureCoords = textureScale * vertexTextureCoords;
}
