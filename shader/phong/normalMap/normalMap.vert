#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};
uniform mat4 model;

out vec3 fragPos;
out vec2 texCoord;
out vec3 Normal;
out mat3 TBN;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1);

	mat3 model3 = mat3(model);
	mat3 normalMatrix = transpose(inverse(model3));

	fragPos = vec3(model * vec4(aPos, 1));
	texCoord = aTexCoord;
	Normal = normalMatrix * aNormal;

	vec3 N = normalize(Normal);
	vec3 T = normalize(normalMatrix * aTangent);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(T, N);
	//vec3 B = cross(N, T);

    TBN = mat3(T, B, N);
}