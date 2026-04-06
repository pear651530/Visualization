#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 60) out;

in VS_OUT {
    vec2 Eigenvector1;
    vec2 Eigenvector2;
    float Eigenvalues1;
    float Eigenvalues2;
    float num;
} gs_in[];

out float fcolor;

const float PI = 3.1415926;

void main() {
	fcolor = gs_in[0].num;

	vec2 radii = vec2(sqrt(gs_in[0].Eigenvalues1),sqrt(gs_in[0].Eigenvalues2)) * 0.04;

	//橢圓歪斜角度計算
	vec2 eigenvector1 = gs_in[0].Eigenvector1;
	float angle = atan(eigenvector1.y / eigenvector1.x);
	if (eigenvector1.x < 0.0)
        	angle += PI;
	// 角度修正为0到2*PI之间
	angle = mod(angle, 2.0 * PI);
	
	for (int i = 0; i < 30; i++) {
		
		//橢圓沒有歪斜狀況下，要畫得點座標(x,y)
		float theta = 2 * PI * float(i) / 60.0;
		float sigma = (gs_in[0].num < 0.5) ? 1 : 2;
		float x = sigma * radii.x * cos(theta);
		float y = sigma * radii.y * sin(theta);
		//橢圓歪斜角度影響(x,y)座標
		float offectx = x * cos(angle) - y * sin(angle);
		float offecty = x * sin(angle) + y * cos(angle);
		gl_Position = gl_in[0].gl_Position + vec4(offectx, offecty, 0, 0);
		EmitVertex();


		theta = 2 * PI * float(60 - i - 1) / 60.0;
		x = sigma * radii.x * cos(theta);
		y = sigma * radii.y * sin(theta);
		offectx = x * cos(angle) - y * sin(angle);
		offecty = x * sin(angle) + y * cos(angle);
		gl_Position = gl_in[0].gl_Position + vec4(offectx, offecty, 0, 0);
		EmitVertex();
	}

	EndPrimitive();
}