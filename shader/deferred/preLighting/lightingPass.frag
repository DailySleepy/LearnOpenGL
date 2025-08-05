#version 460 core

in vec3 lightPos;
in vec3 lightColor;

layout(location = 0) out vec3 diffuseLight;
layout(location = 1) out vec3 specularLight;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;

uniform vec3 viewPos;
uniform float R_max;

void main() {
	vec2 TexCoord = gl_FragCoord.xy / textureSize(gPosition, 0);
	vec3 FragPos = vec3(texture(gPosition, TexCoord));

	vec3 lightVec = lightPos - FragPos;
	float d = length(lightVec);
	if(d > R_max) { discard; }
	float k = 1 / (R_max * R_max);
	float attenuation = 1 - d / R_max;

	vec3 normal = normalize(vec3(texture(gNormal, TexCoord)));

	float ambient = 0.2;

	vec3 lightDir = normalize(lightVec);
	float diff = max(dot(normal, lightDir), 0);
	float diffuse = 0.8 * diff;
	
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	float spec = pow(max(dot(normal, halfwayDir), 0), 32);
	float specular = 1.0 * spec;

	diffuseLight = (ambient + diffuse) * attenuation  * lightColor;
	specularLight = specular * attenuation  * lightColor;
}