#shader vertex
#version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 texCoord;

void main(){
	gl_Position = vec4(aPos, 0, 1);
	texCoord = aTexCoord;
}



#shader fragment
#version 460 core

uniform sampler2D shadowMap;

in vec2 texCoord;

out vec4 FragColor;

void main(){
	float depthValue = texture(shadowMap, texCoord).r;
	FragColor = vec4(vec3(depthValue), 1);
}
