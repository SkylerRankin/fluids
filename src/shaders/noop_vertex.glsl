#version 410 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 uvCoordinate;
out vec2 uv;
uniform mat4 view;
uniform mat4 projection;

void main() {
	uv = uvCoordinate;
	gl_Position = vec4(vertexPosition, 1.0);
};