#version 460 core

struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
	sampler2D normal1;
};

struct PointLight {
	vec3 color;
	vec3 pos;
};

in vec3 Normal;
in vec3 fragPos;
in vec2 texCoord;
in mat3 TBN;

uniform Material material;
uniform PointLight light;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
	vec3 ambient = 0.2 * light.color * vec3(texture(material.diffuse1, texCoord));

	vec3 normal = vec3(texture(material.normal1, texCoord)) * 2.0 - 1.0;
	normal = normalize(TBN * normal);
	vec3 lightDir = normalize(light.pos - fragPos);
	float diff = max(dot(normal, lightDir), 0);
	vec3 diffuse = 0.5 * light.color * diff * vec3(texture(material.diffuse1, texCoord));
	
	vec3 viewDir = normalize(viewPos - fragPos);
	//vec3 reflectDir = reflect(-lightDir, normal);
	//float spec = pow(max(dot(viewDir, reflectDir), 0), 32);
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	float spec = pow(max(dot(normal, halfwayDir), 0), 32);
	vec3 specular = 1.0 * light.color * spec * vec3(texture(material.specular1, texCoord));

	vec3 result = ambient + diffuse + specular;

	FragColor = vec4(result, 1);
}