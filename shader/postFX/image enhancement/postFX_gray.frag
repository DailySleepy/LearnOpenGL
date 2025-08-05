#version 460 core

uniform sampler2D screenTexture;

in vec2 texCoord;

out vec4 FragColor;

void main(){
	vec3 color = vec3(texture(screenTexture, texCoord));
	//color = vec3((0.299 * color.x + 0.587 * color.y + 0.114 * color.z) / 3);
	FragColor = vec4(color, 1);
}
