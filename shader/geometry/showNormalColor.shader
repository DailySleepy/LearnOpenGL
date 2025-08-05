#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

layout(std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

out vec3 normal;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1);
    normal = mat3(transpose(inverse(model))) * aNormal;
}




#shader fragment
#version 460 core

in vec3 normal;

out vec4 FragColor;

void main(){
	FragColor = vec4(normalize(normal) * 0.5 + 0.5, 1.0);
}