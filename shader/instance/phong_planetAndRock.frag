#version 460 core

struct ParallelLight{
	vec3 color;
	vec3 dir;
};

struct PointLight {
	vec3 color;
	vec3 pos;
	vec3 ac; // float c, l, q
};

struct SpotLight {
	vec3 color;
	vec3 pos;
	vec3 dir;
	float cutOff;
	float outerCutOff;
	vec3 ac;
};

struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
	float shininess;
};

in vec3 fragPos;
in vec2 texCoord;
in vec3 Normal;

uniform Material material;
uniform ParallelLight light;
uniform vec3 viewPos;

out vec4 FragColor;

vec3 calculateParallelLight(ParallelLight light, vec3 normal, vec3 viewDir) {
	vec3 ambient = 0.05 * light.color * vec3(texture(material.diffuse1, texCoord));

	vec3 lightDir = normalize(-light.dir);
	float diff = max(dot(normal, lightDir), 0);
	vec3 diffuse = 0.5 * light.color * diff * vec3(texture(material.diffuse1, texCoord));

	return ambient + diffuse;
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 fragPos) {
	vec3 ambient = 0.2 * light.color * vec3(texture(material.diffuse1, texCoord));

	vec3 lightDir = normalize(light.pos - fragPos);
	float diff = max(dot(normal, lightDir), 0);
	vec3 diffuse = 0.5 * light.color * diff * vec3(texture(material.diffuse1, texCoord));
	
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess * 128);
	vec3 specular = 1.0 * light.color * spec * vec3(texture(material.specular1, texCoord));

	float d = length(light.pos - fragPos);
	float attenuation = 1.0 / (light.ac[0] + light.ac[1] * d + light.ac[2] * d * d);

	float theta = dot(lightDir, normalize(-light.dir));
	float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);    

	return (ambient + diffuse * intensity + specular * intensity) * attenuation;
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos) {
	vec3 ambient = 0.2 * light.color * vec3(texture(material.diffuse1, texCoord));

	vec3 lightDir = normalize(light.pos - fragPos);
	float diff = max(dot(normal, lightDir), 0);
	vec3 diffuse = 0.5 * light.color * diff * vec3(texture(material.diffuse1, texCoord));
	
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess * 128);

	vec3 specular = 1.0 * light.color * spec * vec3(texture(material.specular1, texCoord));

	float d = length(light.pos - fragPos);
	float attenuation = 1.0 / (light.ac[0] + light.ac[1] * d + light.ac[2] * d * d);

	return (ambient + diffuse + specular) * attenuation;
}

void main() {
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 result = vec3(0);
	result += calculateParallelLight(light, normal, viewDir);

	FragColor = vec4(result, 1);
}
