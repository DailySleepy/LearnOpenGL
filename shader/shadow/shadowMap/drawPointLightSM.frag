#version 460 core

in vec4 FragPos;

uniform float far_plane;
uniform vec3 lightPos;

void main() {
	float l = length(FragPos.xyz - lightPos);
	l /= far_plane;
	gl_FragDepth = l;
}