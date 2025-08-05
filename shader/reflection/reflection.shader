#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 fragPos;
out vec3 Normal;

void main(){
	fragPos = vec3(model * vec4(aPos, 1));
	Normal = transpose(inverse(mat3(model))) * aNormal;

	gl_Position = projection * view * model * vec4(aPos, 1);
}



#shader fragment
#version 460 core

uniform samplerCube skybox;
uniform vec3 cameraPos;

in vec3 fragPos;
in vec3 Normal;

out vec4 FragColor;

void main(){
	vec3 I = normalize(fragPos - cameraPos);
	vec3 normal = normalize(Normal);
	vec3 R = reflect(I, normal);

	vec3 envMapCoord = vec3(R.x, R.y, R.z);
	FragColor = vec4(vec3(texture(skybox, envMapCoord)), 1);
}

