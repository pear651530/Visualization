#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in float aThickness;
layout (location = 2) in float aSpeed;

out VS_OUT {
    float speedVlaue;
    float thickness;
} vs_out;

uniform float maxSpeed;
uniform float minSpeed;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    	vs_out.speedVlaue = clamp((aSpeed - minSpeed) / (maxSpeed - minSpeed), 0.0, 1.0);//aSpeed / maxSpeed
	vs_out.thickness = aThickness;

	gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
}