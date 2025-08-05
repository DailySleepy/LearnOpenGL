#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT{
    vec2 texCoord;
    vec3 pos;
} gs_in[];

out vec2 gTexCoord;

layout(std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;
uniform float time;

vec3 getNormal(){
    vec3 p0 = vec3(gs_in[0].pos);
    vec3 p1 = vec3(gs_in[1].pos);
    vec3 p2 = vec3(gs_in[2].pos);

    vec3 e1 = p1 - p0;
    vec3 e2 = p2 - p0;

    return normalize(cross(e1, e2));
}

vec4 explode(vec3 position, vec3 normal){
    float magnitude = 2.0;
    vec3 offset = normal * magnitude * ((sin(time) + 1.0) * 0.5);
    //offset = vec3(0);
    return vec4(position + offset, 1);
}

void main() {
    vec3 normal = getNormal();
    for(int i = 0; i < 3; i++){
        gl_Position = projection * view * model * explode(gs_in[i].pos, normal);
        gTexCoord = gs_in[i].texCoord;
        EmitVertex();
    }
    EndPrimitive();
}
