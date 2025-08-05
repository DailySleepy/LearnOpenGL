#version 460 core

struct PointLight {
	vec3 color;
	vec3 pos;
	vec3 ac; // float c, l, q => vec3(1.0, 0.045, 0.0075)
};

struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
};

uniform Material material;
uniform PointLight light;
uniform vec3 viewPos;
uniform samplerCube shadowCubeMap;
uniform int shadowMode;
uniform float near_plane;
uniform float far_plane;
uniform float texelSize;
uniform float lightSizeUV; 
uniform float filterScale;
uniform float EPS;

//#define EPS 0.04
#define PI 3.141592653589793
#define PI2 6.283185307179586

#define NUM_SAMPLES 32
#define NUM_RINGS 10

in vec3 FragPos;
in vec4 FragPosFromLight;
in vec2 TexCoord;
in vec3 Normal;

out vec4 FragColor;

vec3 ambient;
vec3 diffuse;
vec3 specular;
float attenuation;

void blinnPhong(vec3 normal, vec3 viewDir, vec3 fragPos) {
	ambient = 0.2 * light.color * vec3(texture(material.diffuse1, TexCoord));

	vec3 lightDir = normalize(light.pos - fragPos);
	float diff = max(dot(normal, lightDir), 0);
	diffuse = 0.8 * light.color * diff * vec3(texture(material.diffuse1, TexCoord));
	
	vec3 halfVec = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfVec, normal), 0), 32);
	specular = 1.0 * light.color * spec * vec3(texture(material.specular1, TexCoord));

	float d = length(light.pos - fragPos);
	attenuation = 1.0 / (light.ac[0] + light.ac[1] * d + light.ac[2] * d * d);
	attenuation = pow(attenuation, 1/2.4);

	//return ambient + (diffuse + specular) * attenuation;
}

float useShadowMap(vec3 light2frag){
	float visibility = 0.0;

	float sampledDepth = texture(shadowCubeMap, light2frag).r;
	sampledDepth *= far_plane;

	float currentDepth = length(light2frag);

	visibility = currentDepth > sampledDepth + EPS ? 0.0 : 1.0;
	return visibility;
}

const vec2 poissonDisk[NUM_SAMPLES] = {
    vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870), vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554), vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, 0.027550182), vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641307), vec2(0.14383161, -0.14100790),
    vec2(-0.77391045, 0.091353866), vec2(0.89259836, -0.56800054),
    vec2(0.68177972, 0.94226073), vec2(-0.21210893, -0.71707110),
    vec2(0.90087236, 0.10918000), vec2(-0.38997236, -0.93214725),
    vec2(0.85096924, 0.53768248), vec2(-0.60254590, -0.46020549),
    vec2(0.13945398, -0.73223199), vec2(-0.30184295, -0.61705956),
    vec2(0.70943797, -0.19763753), vec2(-0.71736039, 0.61439300),
    vec2(0.51293537, 0.61643672), vec2(-0.31947032, 0.43705956),
    vec2(0.93227851, -0.23588835), vec2(-0.14733660, -0.25127500)
};

float PCF(vec3 light2frag, float filterSize){
	float currentDepth = length(light2frag);

	vec3 uAxis, vAxis;
	vec3 absDir = abs(light2frag);
	float maxAxis = max(absDir.x, max(absDir.y, absDir.z));
	if(maxAxis == absDir.x) { uAxis = vec3(0, 1, 0); vAxis = vec3(0, 0, 1); }
	else if(maxAxis == absDir.y) { uAxis = vec3(1, 0, 0); vAxis = vec3(0, 0, 1); }
	else { uAxis = vec3(1, 0, 0); vAxis = vec3(0, 1, 0); }

	float cosTheta = clamp(dot(normalize(Normal), normalize(-light2frag)), 0.0, 1.0);
	float sinTheta = sqrt(1 - cosTheta * cosTheta);
	float bias = EPS * sinTheta / cosTheta;
	bias = clamp(bias, 0.0, 0.1);

	float visibility = 0.0;
	for(int i = 0; i < NUM_SAMPLES; i++){
		vec2 offset = poissonDisk[i] * filterSize;
		vec3 sampleOffset = uAxis * offset.x + vAxis * offset.y;
		vec3 sampledCoord = light2frag + sampleOffset;
		float sampledDepth = texture(shadowCubeMap, sampledCoord).r;
		sampledDepth *= far_plane;
		visibility += currentDepth > sampledDepth + bias ? 0.0 : 1.0;
	}
	visibility /= float(NUM_SAMPLES);
	return visibility;
}

void main() {
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	blinnPhong(normal, viewDir, FragPos);

	vec3 light2frag = FragPos - light.pos;

	float visibility = 1.0;
	switch (shadowMode) {
        case 0: visibility = 1.0; break;
        case 1: visibility = useShadowMap(light2frag); break;
        case 2: visibility = PCF(light2frag, texelSize * 100 * filterScale); break;
        //case 3: visibility = PCSS(light2frag); break;
		default: visibility = 0.0;
    }

	vec3 color = ambient + (diffuse + specular) * attenuation * visibility;
	FragColor = vec4(color, 1);
}





//highp float rand_3to1(vec3 xyz) {
//	highp float a = 12.9898, b = 78.233, c = 96.1234, d = 43758.5453;
//	highp float dt = dot(xyz, vec3(a, b, c));
//	highp float sn = sin(mod(dt, PI));
//	return fract(sn * d);
//}
//vec2 poissonDisk[NUM_SAMPLES];
//void poissonDiskSamples(vec3 seed) {
//	float angle = rand_3to1(seed) * PI2;
//	float ANGLE_STEP = PI2 * NUM_RINGS / NUM_SAMPLES;
//	float INV_NUM_SAMPLES = 1.0 / NUM_SAMPLES;
//	float radius = INV_NUM_SAMPLES;
//	float RADIUS_STEP = INV_NUM_SAMPLES;
//	for(int i = 0; i < NUM_SAMPLES; i++){
//		poissonDisk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
//		angle += ANGLE_STEP;
//		radius += RADIUS_STEP;
//	}
//}