#version 330 core

uniform vec2 reverseMaxSize;
uniform sampler2D renderTexture;
uniform sampler2D depthTexture;
in vec2 textureCoords;
out vec4 pixelColor;

void main() {
  vec4 baseColor = texture2D(renderTexture, textureCoords);
  float sum = 0.0f;
  float my = texture2D(depthTexture, textureCoords).x;
  sum += texture2D(depthTexture, textureCoords + vec2(+1, 0) * reverseMaxSize).x;
  sum += texture2D(depthTexture, textureCoords + vec2(-1, 0) * reverseMaxSize).x;
  sum += texture2D(depthTexture, textureCoords + vec2(0, +1) * reverseMaxSize).x;
  sum += texture2D(depthTexture, textureCoords + vec2(0, -1) * reverseMaxSize).x;
  float d = abs(sum / my - 4.0f);
  pixelColor = baseColor - vec4(1000.0f * d, 100.0f * d, 10.0f * d, 0);
}
