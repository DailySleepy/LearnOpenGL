#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 5) in mat4 model;
layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};

out vec3 fragPos;
out vec2 texCoord;
out vec3 Normal;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1);

	fragPos = vec3(model * vec4(aPos, 1));
	texCoord = aTexCoord;
	mat3 normalMatrix = transpose(inverse(mat3(model)));
	Normal = normalMatrix * aNormal;
}
