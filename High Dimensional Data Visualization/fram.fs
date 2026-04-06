#version 330 core
out vec4 FragColor;

in float fcolor;

void main()
{
	vec4 color;
	if(fcolor < 0.5) FragColor = vec4(0.0, 0.0, 1.0, 1.0);
	else FragColor = vec4(1.0, 0.0, 0.0, 1.0);
} 