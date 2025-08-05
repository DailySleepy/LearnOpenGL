#version 460 core
#extension GL_ARB_bindless_texture : require

struct Material {
	layout(bindless_sampler) sampler2D diffuse1;
	layout(bindless_sampler) sampler2D specular1;
	layout(bindless_sampler) sampler2D normal1;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform Material material;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

vec3 toLinear(vec3 srgb) {
    vec3 low  = srgb / 12.92;
    vec3 high = pow((srgb + 0.055) / 1.055, vec3(2.4));
    bvec3 cutoff = lessThanEqual(srgb, vec3(0.04045));
    return mix(high, low, cutoff);
}

void main() {
	gPosition = FragPos;
	gNormal = normalize(Normal);
	vec3 color = texture(material.diffuse1, TexCoord).rgb;
	//color = toLinear(color);
	gAlbedoSpec.rgb = color;
	gAlbedoSpec.a   = texture(material.specular1, TexCoord).r;
}