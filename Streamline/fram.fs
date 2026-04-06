#version 330 core
out vec4 FragColor;

in float svalue;

uniform sampler1D T0;

void main()
{
	vec4 color = texture(T0, svalue);
	FragColor = color;
} 