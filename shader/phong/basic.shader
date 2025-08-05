#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};
uniform mat4 model;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1);
	Normal = mat3(transpose(inverse(model))) * aNormal;
	FragPos = vec3(model * vec4(aPos, 1));
	TexCoord = aTexCoord;
}




#shader fragment
#version 460 core
#extension GL_ARB_bindless_texture : require

struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform Material material;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

out vec4 FragColor;

void main() {
	vec3 ambient = 0.2 * lightColor * vec3(texture(material.diffuse1, TexCoord));

	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(normal, lightDir), 0);
	vec3 diffuse = 0.5 * lightColor * diff * vec3(texture(material.diffuse1, TexCoord));
	
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	float spec = pow(max(dot(normal, halfwayDir), 0), 32);
	vec3 specular = 1.0 * lightColor * spec * vec3(texture(material.specular1, TexCoord));

	vec3 result = ambient + diffuse + specular;

	FragColor = vec4(result, 1);
}