#version 460 core

in vec4 Position;
in vec4 Normal;

uniform mat4 ProjectionView;

void main() {
	gl_Position = ProjectionView * Position;
}