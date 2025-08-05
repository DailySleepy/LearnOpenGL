#version 460 core

in vec2 TexCoord;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gAlbedoSpec;

layout(std430, binding = 0) buffer Position { vec3 lightPos[]; };
layout(std430, binding = 1) buffer Color { vec3 lightColor[]; };

const int lightCount = 1000;

uniform vec3 viewPos;
uniform float R_max;

void main() {
	vec3 FragPos = vec3(texture(gPosition, TexCoord));
	vec3 normal = vec3(texture(gNormal, TexCoord));
	vec4 albedo_spec = texture(gAlbedoSpec, TexCoord);
	vec3 albedo = albedo_spec.rgb;
	float spec = albedo_spec.a;

	vec3 result = vec3(0);
	float a = (256 / 5 - 1) / (R_max * R_max);
	 
	for(int i = 0; i < lightCount; i++) {
		vec3 lightDir = normalize(lightPos[i] - FragPos);
		float d = length(lightDir);
		if(d > R_max) continue;
		float attenuation = 1.0 / (a * d * d + 1);

		vec3 ambient = 0.05 * lightColor[i] * albedo;

		float diff = max(dot(normal, lightDir), 0);
		vec3 diffuse = 0.5 * lightColor[i] * diff * albedo;
	
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 halfwayDir = normalize(lightDir + viewDir);  
		spec = pow(max(dot(normal, halfwayDir), 0), 32) * spec;
		vec3 specular = 1.0 * lightColor[i] * spec;

		result += (ambient + diffuse + specular) * attenuation;
	}

	FragColor = vec4(result, 1);
}