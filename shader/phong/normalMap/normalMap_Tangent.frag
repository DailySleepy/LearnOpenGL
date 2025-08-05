#version 460 core

struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
	sampler2D normal1;
};

in vec2 texCoord;
in vec3 lightColor;
in vec3 fragPosTangent;
in vec3 lightPosTangent;
in vec3 viewPosTangent;

uniform Material material;

out vec4 FragColor;

void main() {
	vec3 ambient = 0.2 * lightColor * vec3(texture(material.diffuse1, texCoord));

	vec3 normal = vec3(texture(material.normal1, texCoord)) * 2.0 - 1.0;
	normal = normalize(normal);
	vec3 lightDir = normalize(lightPosTangent - fragPosTangent);
	float diff = max(dot(normal, lightDir), 0);
	vec3 diffuse = 0.5 * lightColor * diff * vec3(texture(material.diffuse1, texCoord));
	
	vec3 viewDir = normalize(viewPosTangent - fragPosTangent);
	//vec3 reflectDir = reflect(-lightDir, normal);
	//float spec = pow(max(dot(viewDir, reflectDir), 0), 32);
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	float spec = pow(max(dot(normal, halfwayDir), 0), 32);
	vec3 specular = 1.0 * lightColor * spec * vec3(texture(material.specular1, texCoord));

	vec3 result = ambient + diffuse + specular;

	FragColor = vec4(result, 1);
}