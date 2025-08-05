#version 460 core

in vec3 lightPos;
in vec3 lightColor;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gAlbedoSpec;

uniform vec3 viewPos;
uniform float R_max;

vec3 toSRGB(vec3 linear) {
    vec3 low  = linear * 12.92;
    vec3 high = 1.055 * pow(linear, vec3(1.0 / 2.4)) - 0.055;
    bvec3 cutoff = lessThanEqual(linear, vec3(0.0031308));
    return mix(high, low, cutoff);
}

void main() {
	vec2 TexCoord = gl_FragCoord.xy / textureSize(gPosition, 0);
	vec3 FragPos = vec3(texture(gPosition, TexCoord));

	vec3 lightVec = lightPos - FragPos;
	float d = length(lightVec);
	if(d > R_max) { discard; }
	float k = 1 / (R_max * R_max);
	float attenuation;
	//attenuation = 1 - k * d * d;
	//attenuation = 1 / (1 + d);
	attenuation = 1 - d / R_max;

	vec3 normal = normalize(vec3(texture(gNormal, TexCoord)));
	vec4 albedo_spec = texture(gAlbedoSpec, TexCoord);
	vec3 albedo = albedo_spec.rgb;
	float spec = albedo_spec.a;

	float ambient = 0.2;

	vec3 lightDir = normalize(lightVec);
	float diff = max(dot(normal, lightDir), 0);
	float diffuse = 0.8 * diff;
	
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	spec = pow(max(dot(normal, halfwayDir), 0), 32) * spec;
	float specular = 1.0 * spec;

	vec3 result = ((ambient + diffuse) * albedo + specular) * attenuation * lightColor;

	FragColor = vec4(result, 1);
}