#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

layout(std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

out vec3 normal;

void main() {
    gl_Position = model * view * vec4(aPos, 1);
    normal = normalize(mat3(transpose(inverse(view * model))) * aNormal);
}




#shader geometry
#version 460 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout(std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};

in vec3 normal[];

float length = 0.03;

void main() {
    for(int i = 0; i < 3; i++){
        gl_Position = projection * gl_in[i].gl_Position;
        EmitVertex();
        gl_Position = projection * (gl_in[i].gl_Position + vec4(normal[i], 0.0) * length);
        EmitVertex();
        EndPrimitive();
    }
}




#shader fragment
#version 460 core

out vec4 FragColor;

void main(){
	FragColor = vec4(1.0, 0.5, 1.0, 1.0);
}