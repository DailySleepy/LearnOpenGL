#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 5) in vec3 aColor;
layout(location = 6) in float id;

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};
uniform mat4 model;

out vec3 color;

void main(){
	gl_Position = projection * view * model * vec4(aPos + vec3(1) * id, 1);
	color = aColor;
}



#shader fragment
#version 460 core

in vec3 color;
out vec4 FragColor;

void main() {
    FragColor = vec4(color, 1.0);
}
