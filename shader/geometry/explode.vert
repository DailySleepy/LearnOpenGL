#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoord;

layout(std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

out VS_OUT{
    vec2 texCoord;
    vec3 pos;
} vs_out;


void main() {
    gl_Position = projection * view * model * vec4(aPos, 1);
    vs_out.texCoord = aTexCoord;
    vs_out.pos = aPos;
}
