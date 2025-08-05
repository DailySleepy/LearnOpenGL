#version 460 core
out vec4 FragColor;
uniform vec4 plane;

void main() {
	FragColor = vec4(vec3(0), 0.5);
	//FragColor = vec4(0.9,0,0,1);
}