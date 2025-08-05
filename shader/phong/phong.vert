#version 460 core

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};
uniform mat4 model;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 fragPos;
out vec2 texCoord;
out vec3 Normal;
out vec3 normalColor;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1);

	mat3 normalMatrix = transpose(inverse(mat3(model)));

	fragPos = vec3(model * vec4(aPos, 1));
	texCoord = aTexCoord;
	Normal = normalMatrix * aNormal;
}
