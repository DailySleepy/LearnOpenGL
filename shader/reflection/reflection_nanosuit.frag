#version 460 core

struct Material {
	sampler2D diffuse1;
	sampler2D ambient1;
};

uniform Material material;
uniform samplerCube skybox;
uniform vec3 cameraPos;

in vec3 fragPos;
in vec3 Normal;
in vec2 texCoord;

out vec4 FragColor;

void main(){
	vec3 diffuse = texture(material.diffuse1, texCoord).rgb;

	vec3 I = normalize(fragPos - cameraPos);
	vec3 normal = normalize(Normal);
	vec3 R = reflect(I, normal);
	vec3 envColor = texture(skybox, R).rgb;

	float reflectionValue = texture(material.ambient1, texCoord).r; 

	//float factor = smoothstep(0.0, 1.0, reflectionValue);
	//FragColor = vec4(mix(diffuse, envColor, factor), 1.0);

	vec3 reflection = envColor * reflectionValue;
	FragColor = vec4((diffuse + reflection), 1);
}
