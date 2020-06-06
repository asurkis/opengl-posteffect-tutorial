#version 330 core

out vec4 pixelColor;

void main() {
  pixelColor = vec4(vec3(exp(-gl_FragCoord.w)), 1);
}
