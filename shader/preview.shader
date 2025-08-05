#shader vertex
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout(std140, binding = 0) uniform Matrices{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

out vec2 texCoord;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    vec2 texScale = vec2(1);
    texCoord = aTexCoord * texScale;
}



#shader fragment
#version 460 core

struct Material {
	sampler2D diffuse1;
};

out vec4 FragColor;

in vec2 texCoord;

uniform Material material;

void main()
{    
    FragColor = texture(material.diffuse1, texCoord);
}