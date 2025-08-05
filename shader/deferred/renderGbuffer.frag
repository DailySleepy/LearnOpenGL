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

void main() {
	gPosition = FragPos;
	gNormal = normalize(Normal);
	gAlbedoSpec.rgb = texture(material.diffuse1, TexCoord).rgb;
	gAlbedoSpec.a   = texture(material.specular1, TexCoord).r;
}