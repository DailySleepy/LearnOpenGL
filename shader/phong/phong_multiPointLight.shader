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

struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

layout(std430, binding = 0) buffer LightPosBuffer { vec3 lightPos[]; };
layout(std430, binding = 1) buffer LightColorBuffer { vec3 lightColor[]; };

uniform Material material;
uniform vec3 viewPos;
uniform int lightCount;
uniform float R_max;

out vec4 FragColor;

void main() {
	vec3 albedo = texture(material.diffuse1, TexCoord).rgb;
	float alpha = texture(material.diffuse1, TexCoord).a;
	float spec = texture(material.specular1, TexCoord).r; spec = 0.5;
	vec3 normal = normalize(Normal);

	vec3 result = vec3(0);

	for(int i = 0; i < lightCount; i++) 
	{
		vec3 lightVec = lightPos[i] - FragPos;
		float d = length(lightVec);
		if(d > R_max) { continue; }

		float attenuation = 1 - d / R_max;

		float ambient = 0.2;

		vec3 lightDir = normalize(lightVec);
		float diff = max(dot(normal, lightDir), 0);
		float diffuse = 0.8 * diff;
	
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 halfwayDir = normalize(lightDir + viewDir);  
		float specular = pow(max(dot(normal, halfwayDir), 0), 32) * spec;

		result += ((ambient + diffuse) * albedo + specular) * lightColor[i] * attenuation;
	}

	FragColor = vec4(result, alpha);
}