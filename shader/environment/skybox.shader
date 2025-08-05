#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;

layout(std140) uniform Matrices{
	uniform mat4 projection;
	uniform mat4 view;
};

out vec3 dir;

void main(){
	dir = aPos;
	vec4 pos = projection * view * vec4(aPos, 1);
	gl_Position = pos.xyww;
}



#shader fragment
#version 460 core

uniform samplerCube skybox;

in vec3 dir;

out vec4 FragColor;

void main(){
	vec3 envMapCoord = vec3(dir.x, dir.y, dir.z);
	FragColor = texture(skybox, envMapCoord);
}
