#version 460 core
layout(location = 0) in vec3 aPos;
layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};
uniform mat4 prevProjection;
uniform mat4 prevView;
uniform mat4 model;

out vec4 currPos;
out vec4 prevPos;

void main(){
	currPos = projection * view * model * vec4(aPos, 1);
	prevPos = prevProjection * prevView * model * vec4(aPos, 1);
	gl_Position = currPos;
}