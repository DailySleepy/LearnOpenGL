#version 460 core

struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
	sampler2D normal1;
	sampler2D height1;
};

in vec2 TexCoord;
in vec3 fragPosTangent;
in vec3 viewPosTangent;
in vec3 lightPosTangent;

uniform Material material;
uniform vec3 lightColor;
uniform float heightScale;
uniform bool divideByZ;
uniform int parallaxMode;

out vec4 FragColor;

vec2 ParallaxMapping(vec2 coord, vec3 viewDir) {
	vec2 p = viewDir.xy * texture(material.height1, coord).r * heightScale;
	if(divideByZ) p /= (viewDir.z + 0.1);
	return coord - p;
}

float minLayers = 8;
uniform float maxLayers;

vec2 SteepParallaxMapping(vec2 coord, vec3 viewDir) {
	float height = texture(material.height1, coord).r;
	vec2 p = viewDir.xy * height * heightScale;
	if(divideByZ) p /= (viewDir.z + 0.1);
	
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), viewDir)));
	float layerDepth = 1.0 / numLayers;
	vec2 deltaUV = p / numLayers;
	vec2 uv = coord;

	float currentLayerDepth = 0.0;
	float currentDepth = height;
	while(currentLayerDepth < currentDepth) {
		uv -= deltaUV;
		currentLayerDepth += layerDepth;
		currentDepth = texture(material.height1, uv).r;
	}
	return uv;
}

vec2 ParallaxOcclusionMapping(vec2 coord, vec3 viewDir) {
	float height = texture(material.height1, coord).r;
	vec2 p = viewDir.xy * height * heightScale;
	if(divideByZ) p /= (viewDir.z + 0.1);

	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), viewDir)));
	float layerDepth = 1.0 / numLayers;
	vec2 deltaUV = p / numLayers;
	vec2 currentUV = coord;

	float currentLayerDepth = 0.0;
	float currentDepth = height;
	while(currentLayerDepth < currentDepth) {
		currentUV -= deltaUV;
		currentLayerDepth += layerDepth;
		currentDepth = texture(material.height1, currentUV).r;
	}
	float deltaC = currentLayerDepth - currentDepth;

	vec2 prevUV = currentUV + deltaUV;
	float prevLayerDepth = currentLayerDepth - layerDepth;
	float prevDepth = texture(material.height1, prevUV).r;
	float deltaP = prevDepth - prevLayerDepth;

	float w = deltaC / (deltaC + deltaP + 0.1);

	return mix(currentUV, prevUV, w);
}

void main() {
	vec3 viewDir = normalize(viewPosTangent - fragPosTangent);
	vec2 texCoord;

    switch (parallaxMode) {
        case 0: texCoord = TexCoord; break;
        case 1: texCoord = ParallaxMapping(TexCoord, viewDir); break;
        case 2: texCoord = SteepParallaxMapping(TexCoord, viewDir); break;
		case 3: texCoord = ParallaxOcclusionMapping(TexCoord, viewDir); break;
		default: texCoord = vec2(0, 0);
    }

	vec3 ambient = 0.2 * lightColor * vec3(texture(material.diffuse1, texCoord));

	//vec3 normal = vec3(0.0, 0.0, 1.0);
	vec3 normal = normalize(texture(material.normal1, texCoord).rgb * 2.0 - 1.0);
	vec3 lightDir = normalize(lightPosTangent - fragPosTangent);
	float diff = max(dot(normal, lightDir), 0);
	vec3 diffuse = 0.5 * lightColor * diff * vec3(texture(material.diffuse1, texCoord));
	
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	float spec = pow(max(dot(normal, halfwayDir), 0), 32);
	vec3 specular = 1.0 * lightColor * spec * vec3(texture(material.specular1, texCoord));

	vec3 result = ambient + diffuse + specular;

	FragColor = vec4(result, 1);
}