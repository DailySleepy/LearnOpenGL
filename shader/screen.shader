#shader vertex
#version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main(){
	gl_Position = vec4(aPos, 0.0, 1.0);
	TexCoord = aTexCoord;
}



#shader fragment
#version 460 core

uniform sampler2D screenTexture;

in vec2 TexCoord;

out vec4 FragColor;

void main(){
	FragColor = texture(screenTexture, TexCoord);
	//FragColor = vec4(1.0, 0.5, 0.5, 1);
}
