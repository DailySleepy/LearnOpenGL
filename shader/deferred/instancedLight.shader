#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 5) in vec3 aInstancePos;
layout(location = 6) in vec3 aColor;

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};

uniform float radius;

out vec3 color;

void main(){
	vec3 worldPos = aPos * radius + aInstancePos;
	gl_Position = projection * view * vec4(worldPos, 1);
	color = aColor;
}



#shader fragment
#version 460 core

in vec3 color;

out vec4 FragColor;

void main(){
	FragColor = vec4(color, 1);
}
