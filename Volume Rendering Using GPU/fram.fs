#version 330 core
out vec4 FragColor;

struct Material {  
    sampler2D diffuse;
    vec3 specular;    
    float shininess;
}; 

struct Light {
    //vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 TextureCoord; 
  
uniform vec3 viewPos;//eye pos
uniform Material material;
uniform Light light;

uniform vec3 raydir;
uniform sampler3D T0;
uniform sampler1D T1;
uniform int check_phong;

vec3 phong_shade(vec3 norm, vec3 currentPos, vec3 objectColor, vec3 texCoord) {
	// ambient
	vec3 ambient = light.ambient * texCoord;// * texCoord
  	
	// diffuse 
	// vec3 lightDir = normalize(light.position - currentPos);
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texCoord;// * texCoord 
    
	// specular
	vec3 viewDir = normalize(viewPos - currentPos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * (spec * material.specular);  
        
	vec3 result = (ambient + diffuse + specular) * objectColor;

	return result;
}

void main()
{
	vec3 d;//ray dir
	float T;//acc. opacities
	vec3 texCoord, Color;
	vec3 myColor;
	vec4 texel0, texel1;
	vec3 currentPos = FragPos;

	d = normalize(raydir);//normalize(FragPos - viewPos)
	T = 0.0;
	Color = vec3(0.0);
	texCoord = TextureCoord;

	while(true) {
		texel0 = texture(T0, texCoord.xyz);
		texel1 = texture(T1, texel0.a);
		vec3 N = texture(T0, texCoord.xyz).xyz;
		N = normalize(2.0 * N - 1.0);
		if(check_phong == 1) myColor = phong_shade(N, currentPos, texel1.xyz, texCoord) * texel1.a;
		else myColor = texel1.xyz * texel1.a;
		Color = Color + (1-T) * myColor;
		T = T + (1-T) * (texel1.a);// + ((check_phong == 1) ? 5.0f : 0.0f)

		currentPos = currentPos + 0.15f * d;
		texCoord = texCoord + 0.15f * d / 255.0f;//texCoord = TextureCoord + (currentPos - FragPos) / 255.0f

		if(texCoord.x < 0 || texCoord.x > 1 || texCoord.y < 0 || texCoord.y > 1 || texCoord.z < 0 || texCoord.z > 1) break;//超出texture範圍=超出view volume範圍
		if(T > 0.99) break;//過飽和
	}

	
	FragColor = vec4(Color, min(T, 1.0));
} 