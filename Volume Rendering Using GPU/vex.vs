#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aTexture;

out vec3 FragPos;
out vec3 TextureCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	FragPos = (model * vec4(aPos, 1.0)).xyz;
	TextureCoord = aTexture;//(model * vec4(aTexture, 1.0)).xyz
	//TextureCoord = vec3(1.0 - aTexture.x, aTexture.y, 1.0 - aTexture.z);
    
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}