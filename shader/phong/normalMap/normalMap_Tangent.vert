#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

struct PointLight {
	vec3 color;
	vec3 pos;
};

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};
uniform mat4 model;
uniform PointLight light;
uniform vec3 viewPos;

out vec2 texCoord;
out vec3 lightColor;
out vec3 fragPosTangent;
out vec3 lightPosTangent;
out vec3 viewPosTangent;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1);

	mat3 model3 = mat3(model);
	mat3 normalMatrix = transpose(inverse(model3));
	vec3 N = normalize(normalMatrix * aNormal);
	vec3 T = normalize(normalMatrix * aTangent);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(T, N);
	//vec3 B = cross(N, T);
    mat3 TBN_inv = transpose(mat3(T, B, N));

	texCoord = aTexCoord;
	lightColor = light.color;
	fragPosTangent = TBN_inv * vec3(model * vec4(aPos, 1));
	lightPosTangent = TBN_inv * light.pos;
	viewPosTangent = TBN_inv * viewPos;
}
