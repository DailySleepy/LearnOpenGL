#version 460 core

in vec2 TexCoord;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D diffuseLight;
layout(binding = 1) uniform sampler2D specularLight;
layout(binding = 2) uniform sampler2D gAlbedoSpec;

vec3 toSRGB(vec3 linear) {
    vec3 low  = linear * 12.92;
    vec3 high = 1.055 * pow(linear, vec3(1.0 / 2.4)) - 0.055;
    bvec3 cutoff = lessThanEqual(linear, vec3(0.0031308));
    return mix(high, low, cutoff);
}

void main() {
	vec3 diffuse = texture(diffuseLight, TexCoord).rgb;
	vec3 specular = texture(specularLight, TexCoord).rgb;
	vec4 albedoSpec = texture(gAlbedoSpec, TexCoord);
	vec3 albedo = albedoSpec.rgb;
	float spec  = albedoSpec.a;

	vec3 color = diffuse * albedo + specular * spec;
	//color = toSRGB(color);
	FragColor = vec4(color, 1);
}