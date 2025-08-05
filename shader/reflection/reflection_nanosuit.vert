#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 fragPos;
out vec3 Normal;
out vec2 texCoord;

void main(){
	fragPos = vec3(model * vec4(aPos, 1));
	Normal = transpose(inverse(mat3(model))) * aNormal;
	texCoord = aTexCoord;

	gl_Position = projection * view * model * vec4(aPos, 1);
}
