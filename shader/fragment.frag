#version 330

uniform sampler2D Tex;

out vec4 FragColor;

in vec3 Color;
in vec2 TexCoord;
in vec2 Position;

void main() {

	vec4 texColor = texture(Tex, TexCoord);
	vec3 mixColor =mix(vec3(texColor), Color, 0.3);
	FragColor = vec4(mixColor, 1.0);
}
