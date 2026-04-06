#version 330 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT {
    float speedVlaue;
    float thickness;
} gs_in[];

out float svalue;

uniform float user_mod_thick;

void main() {
	float width = gs_in[0].thickness;
	
	vec4 line = gl_in[1].gl_Position - gl_in[0].gl_Position;
	vec3 direction = normalize(cross(line.xyz, vec3(0.0, 0.0, 1.0)));
	vec4 offset = vec4(direction, 0.0) * 0.006 * user_mod_thick * width;

	//用矩形畫線條
	svalue = gs_in[0].speedVlaue;
	gl_Position = gl_in[0].gl_Position - offset / 2;
	EmitVertex();

	svalue = gs_in[1].speedVlaue;
	gl_Position = gl_in[1].gl_Position - offset / 2;
	EmitVertex();
	
	svalue = gs_in[0].speedVlaue;
	gl_Position = gl_in[0].gl_Position + offset / 2;
	EmitVertex();

	svalue = gs_in[1].speedVlaue;
	gl_Position = gl_in[1].gl_Position + offset / 2;
	EmitVertex();

	EndPrimitive();
}