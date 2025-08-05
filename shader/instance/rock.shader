#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoord;
layout(location = 5) in mat4 model;

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};
//uniform mat4 model;

out vec2 texCoord;

void main(){
	gl_Position = projection * view * model * vec4(aPos, 1);
	texCoord = aTexCoord;
}




#shader fragment
#version 460 core

struct Material{
	sampler2D diffuse1;
};

uniform Material matrial;

in vec2 texCoord;

out vec4 FragColor;

void main(){
	FragColor = vec4(vec3(texture(matrial.diffuse1, texCoord)), 1);
}