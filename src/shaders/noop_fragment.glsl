#version 410 core

in vec2 uv;
out vec4 outColor;
uniform sampler2D gridTexture;

void main() {
    outColor = vec4(0.0, 0.4, 0.6, 1.0);
    outColor = vec4(uv.x, uv.y, 0.0, 1.0);
    outColor = vec4(texture(gridTexture, uv).rgb, 1.0);
};