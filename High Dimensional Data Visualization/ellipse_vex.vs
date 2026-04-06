#version 330 core
layout (location = 0) in vec2 aPu;
layout (location = 1) in vec2 aEigenvector1;
layout (location = 2) in vec2 aEigenvector2;
layout (location = 3) in float aEigenvalues1;
layout (location = 4) in float aEigenvalues2;
layout (location = 5) in float aNum;

out VS_OUT {
    vec2 Eigenvector1;
    vec2 Eigenvector2;
    float Eigenvalues1;
    float Eigenvalues2;
    float num;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vs_out.num = aNum;
	vs_out.Eigenvector1 = aEigenvector1;
	vs_out.Eigenvector2 = aEigenvector2;
	vs_out.Eigenvalues1 = aEigenvalues1;
	vs_out.Eigenvalues2 = aEigenvalues2;
	gl_Position = projection * view * model * vec4(aPu, 0.0, 1.0);
}