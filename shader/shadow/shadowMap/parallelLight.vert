#version 460 core

layout(std140, binding = 0) uniform Matrices {
    mat4 projection;
    mat4 view;
};
layout(std140, binding = 1) uniform LightMatrices {
    mat4 lightProjection;
    mat4 lightView;
};

uniform mat4 model;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec4 FragPosFromLight;
out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1);
    FragPosFromLight = lightProjection * lightView * model * vec4(aPos, 1);
    FragPos = vec3(model * vec4(aPos, 1));
    TexCoord = aTexCoord;
    Normal = transpose(inverse(mat3(model))) * aNormal;
}