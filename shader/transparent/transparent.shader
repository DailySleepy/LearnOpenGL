#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 texCoord;

void main(){
	gl_Position = projection * view * model * vec4(aPos, 1);
	texCoord = aTexCoord;
}



#shader fragment
#version 460 core

uniform sampler2D texture1;

in vec2 texCoord;

out vec4 FragColor;

void main(){
	vec4 color = texture(texture1, texCoord);
	if(color.a < 0.05) discard;
	FragColor = color;
}
