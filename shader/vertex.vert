#version 330

layout (location = 0)in vec2 VertexPosition;
layout (location = 1)in vec2 inCoord;

uniform float time;

out vertexdata {
	vec2 TexCoord;
	vec2 Position;
} vertex;


void main() {
	vertex.TexCoord = inCoord;
	vertex.Position = VertexPosition;
	gl_Position = vec4(VertexPosition,0.0, 1.0);

}
