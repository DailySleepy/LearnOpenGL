#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};
uniform mat4 model;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec2 TexCoord;
out vec3 fragPosTangent;
out vec3 viewPosTangent;
out vec3 lightPosTangent;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1);

	mat3 normalMatrix = mat3(transpose(inverse(model)));

    vec3 N = normalize(normalMatrix * aNormal);
	vec3 T = normalize(normalMatrix * aTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(T, N);
	mat3 TBN_inv = transpose(mat3(T, B, N));

	//TexCoord = aTexCoord;
	TexCoord = vec2(aTexCoord.x, 1 - aTexCoord.y);
	fragPosTangent = TBN_inv * vec3(model * vec4(aPos, 1));
	lightPosTangent = TBN_inv * lightPos;
	viewPosTangent = TBN_inv * viewPos;
}