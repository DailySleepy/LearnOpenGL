#version 460 core

struct ParallelLight{
	vec3 color;
	vec3 dir;
};

struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
};

uniform Material material;
uniform ParallelLight light;
uniform vec3 viewPos;
uniform sampler2D shadowMap;
uniform int shadowMode;
uniform float texelSize;
uniform float lightSizeUV; 
uniform float filterScale;

#define EPS 0.004
#define PI 3.141592653589793
#define PI2 6.283185307179586

#define NUM_SAMPLES 100
#define NUM_RINGS 10
#define NEAR 0.1
#define FAR 12.0

in vec3 FragPos;
in vec4 FragPosFromLight;
in vec2 TexCoord;
in vec3 Normal;

out vec4 FragColor;

vec3 ambient;
vec3 diffuse;
vec3 specular;

void blinnPhong(vec3 normal, vec3 viewDir){
	ambient = 0.15 * light.color * vec3(texture(material.diffuse1, TexCoord));

	vec3 lightDir = normalize(-light.dir);
	float diff = max(dot(normal, lightDir), 0);
	diffuse = 0.5 * light.color * diff * vec3(texture(material.diffuse1, TexCoord));
	
	vec3 halfVec = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfVec, normal), 0), 32);
	specular = 1.0 * light.color * spec * vec3(texture(material.specular1, TexCoord));
}

float useShadowMap(vec3 coord){
	float visibility = 0.0;
	float currentDepth = coord.z;
	float sampledDepth = texture(shadowMap, coord.xy).r;
	visibility = currentDepth > sampledDepth + EPS ? 0.0 : 1.0;
	return visibility;
}

highp float rand_2to1(vec2 uv) {
	highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot(uv, vec2(a, b));
	highp float sn = sin(mod(dt, PI));
	return fract(sn * c);
}

vec2 poissonDisk[NUM_SAMPLES];

void poissonDiskSamples(vec2 seed){
	float angle = rand_2to1(seed) * PI2;
	float ANGLE_STEP = PI2 * NUM_RINGS / NUM_SAMPLES;
	float INV_NUM_SAMPLES = 1.0 / NUM_SAMPLES;
	float radius = INV_NUM_SAMPLES;
	float RADIUS_STEP = INV_NUM_SAMPLES;

	for(int i = 0; i < NUM_SAMPLES; i++){
		poissonDisk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
		angle += ANGLE_STEP;
		radius += RADIUS_STEP;
	}
}

float PCF(vec3 coord, float filterSize){
	poissonDiskSamples(coord.xy);
	float currentDepth = coord.z;

	float visibility = 0.0;
	for(int i = 0; i < NUM_SAMPLES; i++){
		vec2 offset = poissonDisk[i] * filterSize;
		vec2 sampledCoord = coord.xy + offset;
		float sampledDepth = texture(shadowMap, sampledCoord).r;
		visibility += currentDepth > sampledDepth + EPS ? 0.0 : 1.0;
	}
	visibility /= float(NUM_SAMPLES);
	return visibility;
}

float getAverageBlockerDepth(vec2 uv, float receiverDepth){
	poissonDiskSamples(uv);
	float depthSum = 0.0, depth = 0.0;
	int count = 0;
	float searchRadius = lightSizeUV * (1 - (receiverDepth - NEAR) / FAR);
	for(int i = 0; i < NUM_SAMPLES; i++){
		vec2 offset = poissonDisk[i] * searchRadius;
		vec2 sampledCoord = uv + offset;
		float sampledDepth = texture(shadowMap, sampledCoord).r;
		if(receiverDepth > sampledDepth + EPS){
			depthSum += sampledDepth;
			count++;
		}
	}
	depth = count == 0 ? -1 : depthSum / count;
	return depth;
}

float PCSS(vec3 coord){
	 float receiverDepth = coord.z;
	float blockerDepth = getAverageBlockerDepth(coord.xy, receiverDepth);
	if(blockerDepth < 0) return 1;

	float penumbra = (receiverDepth - blockerDepth) / blockerDepth * lightSizeUV;
	float filterSize = penumbra * filterScale;

	float visibility = PCF(coord, filterSize);
	return visibility;
}

void main() {
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);
	blinnPhong(normal, viewDir);

	vec3 coord = (FragPosFromLight.xyz / FragPosFromLight.w) * 0.5 + 0.5;

	float visibility = 1.0;
	switch (shadowMode) {
       case 0: visibility = 1.0; break;
       case 1: visibility = useShadowMap(coord); break;
       case 2: visibility = PCF(coord, texelSize * 5 * filterScale); break;
       case 3: visibility = PCSS(coord); break;
		default: visibility = 0.0;
    }

	vec3 color = ambient + (diffuse + specular) * visibility;
	FragColor = vec4(color, 1);
}