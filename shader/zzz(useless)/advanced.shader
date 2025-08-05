#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out VertexData{
	vec2 texCoord;
} v_out;

void main(){
	v_out.texCoord = aTexCoord;

	gl_Position = projection * view * model * vec4(aPos, 1);
}



#shader fragment
#version 460 core

layout (depth_greater) out float gl_FragDepth;

uniform sampler2D frontTex;

in VertexData{
	vec2 texCoord;
} f_in;

out vec4 FragColor;

void main(){
	FragColor = vec4(texture(frontTex, f_in.texCoord).rgb, 1); 
	gl_FragDepth = gl_FragCoord.z + 0.02;
}
