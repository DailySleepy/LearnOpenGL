#version 460 core

in vec2 TexCoord;

out vec4 FragColor;

layout(std430, binding = 0) buffer LightPosBuffer { vec3 lightPos[]; };
layout(std430, binding = 1) buffer LightColorBuffer { vec3 lightColor[]; };
layout(std430, binding = 2) buffer LightIndiceBuffer { int lightIndices[]; };

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gAlbedoSpec;

uniform vec3 viewPos;
uniform int tileSize;
uniform int tileCountX;
uniform int maxLightsPerTile;
uniform float R_max;

void main() {
	vec3 normal = vec3(texture(gNormal, TexCoord));

	if(normal == vec3(0)) {
		FragColor = vec4(vec3(0), 1);
		return;
	}

	normal = normalize(normal);
	vec3 FragPos = vec3(texture(gPosition, TexCoord));
	vec4 albedo_spec = texture(gAlbedoSpec, TexCoord);
	vec3 albedo = albedo_spec.rgb;
	float spec = albedo_spec.a;

	ivec2 tileId = ivec2(gl_FragCoord.xy / float(tileSize)); // clamp is necessary
	int tileIndex = tileId.y * tileCountX + tileId.x;
	int indexOffset  = tileIndex * maxLightsPerTile;
	int lightCountInTile = lightIndices[indexOffset];

	vec3 result = vec3(0);

	for(int i = 1; i <= lightCountInTile; i++) 
	{
		int index = lightIndices[indexOffset + i];

		vec3 lightVec = lightPos[index] - FragPos;
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

		result += ((ambient + diffuse) * albedo + specular) * lightColor[index] * attenuation;
		//result += lightColor[index];
	}

	FragColor = vec4(result, 1);
}