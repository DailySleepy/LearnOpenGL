#version 460 core
#extension GL_ARB_bindless_texture : require

in vec2 TexCoord;

out vec4 FragColor;

layout(std430, binding = 0) buffer LightPosBuffer { vec3 lightPos[]; };
layout(std430, binding = 1) buffer LightColorBuffer { vec3 lightColor[]; };
layout(std430, binding = 2) buffer LightIndiceBuffer { int lightIndices[]; };

layout(bindless_sampler, binding = 0) uniform sampler2D gPosition;
layout(bindless_sampler, binding = 1) uniform sampler2D gNormal;
layout(bindless_sampler, binding = 2) uniform sampler2D gAlbedoSpec;
layout(bindless_sampler, binding = 3) uniform sampler2D gDepth;

uniform vec3 viewPos;
uniform int tileSize;
uniform int tileCountX;
uniform int tileCountY;
uniform int depthSliceCount;
uniform int maxLightsPerCluster;
uniform float R_max;
uniform float z_near;
uniform float z_far;
uniform bool enableLogDepthSlice;

float ndc2LinearRatio(float z_ndc) {
	float A = (z_far + z_near) / (z_near - z_far);
    float B = (2.0 * z_far * z_near) / (z_near - z_far);
    float z_view = B / -(z_ndc + A);
	float t;
	if(enableLogDepthSlice) {
		float log_near = log(z_near + 1.0);
		float log_far  = log(z_far + 1.0);
		t = (log(-z_view + 1.0) - log_near) / (log_far - log_near);
	} else {
		t = (-z_view - z_near) / (z_far - z_near);
	}
    return clamp(t, 0.0, 1.0);
}

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

	int tileX = int(gl_FragCoord.x / float(tileSize));
	int tileY = int(gl_FragCoord.y / float(tileSize));
	float z_buffer = texture(gDepth, TexCoord).r;
	float z_ndc = z_buffer * 2. - 1.;
	float t = ndc2LinearRatio(z_ndc);
	int clusterZ = int(t * depthSliceCount);
	int clusterIndex = tileX + tileCountX * (tileY + tileCountY * clusterZ);

	int indexOffset  = clusterIndex * maxLightsPerCluster;
	int lightCountInCluster = lightIndices[indexOffset];

	vec3 result = vec3(0);

	for(int i = 1; i <= lightCountInCluster; i++) 
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
	}

	FragColor = vec4(result, 1);
}