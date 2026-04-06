#version 330 core
out vec4 FragColor;

struct Material {  
    sampler2D diffuse;
    sampler2D specular;    
    float shininess;
}; 

struct Light {
    //vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 viewPos;//eye pos
uniform Material material;
uniform Light light;
uniform int mesh;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

vec3 phong_shade(vec3 norm, vec3 currentPos, vec2 texcoords) {
	// ambient
	vec3 ambient = light.ambient * texture(material.diffuse, texcoords).rgb;
  	
	// diffuse 
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texcoords).rgb;
    
	// specular
	vec3 viewDir = normalize(viewPos - currentPos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, texcoords).rgb;  
        
	vec3 result = ambient + diffuse + specular;

	return result;
}

void main()
{
	vec3 myColor;
	if(mesh == 1) myColor = glm::vec3(1.0, 1.0, 1.0);
	else myColor = phong_shade(Normal, FragPos, TexCoords);

	FragColor = vec4(myColor, 1.0);
} 