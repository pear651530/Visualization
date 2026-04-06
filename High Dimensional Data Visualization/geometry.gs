#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 60) out;

in float Fraudulent[];
out float fcolor;

const float PI = 3.1415926;

void main() {
	fcolor = Fraudulent[0];
	for (int i = 0; i < 30; i++) {

		// 每邊之間的角度，以弧度計
        	float ang = PI * 2.0 / 10.0 * i;
        	vec4 offset = vec4(cos(ang) * 0.005, -sin(ang) * 0.005, 0.0, 0.0);
        	gl_Position = gl_in[0].gl_Position + offset;
        	EmitVertex();

		// 每邊之間的角度，以弧度計
        	ang = PI * 2.0 / 10.0 * (60 - i - 1);
        	offset = vec4(cos(ang) * 0.005, -sin(ang) * 0.005, 0.0, 0.0);
        	gl_Position = gl_in[0].gl_Position + offset;
        	EmitVertex();
	}

	EndPrimitive();
}