#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;

layout(std140) uniform LightMatrices{
    mat4 lightProjection;
    mat4 lightView;
};
uniform mat4 model;

void main()
{
    gl_Position = lightProjection * lightView * model * vec4(aPos, 1.0);
}



#shader fragment
#version 460 core

out vec4 FragColor;

void main()
{    
    // gl_FragDepth = gl_FragCoord.z;
}