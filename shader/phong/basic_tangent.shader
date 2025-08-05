#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};
uniform mat4 model;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec2 texCoord;
out vec3 fragPosTangent;
out vec3 viewPosTangent;
out vec3 lightPosTangent;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1);

	mat3 normalMatrix = mat3(transpose(inverse(model)));

    vec3 N = normalize(normalMatrix * aNormal);
	vec3 T = normalize(normalMatrix * aTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(T, N);
	mat3 TBN_inv = transpose(mat3(T, B, N));

	texCoord = aTexCoord;
	fragPosTangent = TBN_inv * vec3(model * vec4(aPos, 1));
	lightPosTangent = TBN_inv * lightPos;
	viewPosTangent = TBN_inv * viewPos;
}



#shader fragment
#version 460 core

struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
};

in vec2 texCoord;
in vec3 fragPosTangent;
in vec3 viewPosTangent;
in vec3 lightPosTangent;

uniform Material material;
uniform vec3 lightColor;

out vec4 FragColor;

void main() {
	vec3 ambient = 0.2 * lightColor * vec3(texture(material.diffuse1, texCoord));

	vec3 normal = vec3(0.0, 0.0, 1.0);
	vec3 lightDir = normalize(lightPosTangent - fragPosTangent);
	float diff = max(dot(normal, lightDir), 0);
	vec3 diffuse = 0.5 * lightColor * diff * vec3(texture(material.diffuse1, texCoord));
	
	vec3 viewDir = normalize(viewPosTangent - fragPosTangent);
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	float spec = pow(max(dot(normal, halfwayDir), 0), 32);
	vec3 specular = 1.0 * lightColor * spec * vec3(texture(material.specular1, texCoord));

	vec3 result = ambient + diffuse + specular;

	FragColor = vec4(result, 1);
}