#version 330 core

uniform vec2 reverseMaxSize;
uniform sampler2D renderTexture;
uniform sampler2D depthTexture;
in vec2 textureCoords;
out vec4 pixelColor;

void main() {
  vec4 baseColor = texture2D(renderTexture, textureCoords);
  pixelColor = vec4(baseColor.x, 1 - baseColor.y, baseColor.z, 1);
}
