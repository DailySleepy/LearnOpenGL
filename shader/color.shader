#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};
uniform mat4 model;

void main(){
	gl_Position = projection * view * model * vec4(aPos, 1);
}



#shader fragment
#version 460 core

uniform vec3 iColor;

out vec4 FragColor;

vec3 toneMap(vec3 color) {
	//color = pow(color, vec3(1.0/2.4));
    return color;
}

void main(){
	vec3 color = toneMap(iColor);
	FragColor = vec4(color, 1);
}
