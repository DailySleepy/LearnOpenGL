#version 460 core

layout(location = 0) in vec3 aPos;

layout(std140) uniform Matrices {
	mat4 projection;
	mat4 view;
};
uniform mat4 model;
uniform vec3 lightPos;
uniform vec4 plane;

void main() {
	float dotNL = dot(plane.xyz, lightPos);
    float dotNLPd = dotNL + plane.w;

	mat4 shadowProj = mat4(
		plane.x * lightPos.x - dotNLPd, plane.x * lightPos.y, plane.x * lightPos.z, plane.x,
        plane.y * lightPos.x, plane.y * lightPos.y - dotNLPd, plane.y * lightPos.z, plane.y,
        plane.z * lightPos.x, plane.z * lightPos.y, plane.z * lightPos.z - dotNLPd, plane.z,
        plane.w * lightPos.x, plane.w * lightPos.y, plane.w * lightPos.z, -dotNL
	);
	vec4 worldPos = model * vec4(aPos, 1);
	vec4 shadowPos = shadowProj * worldPos;
	shadowPos /= shadowPos.w; // important!!!!!

	gl_Position = projection * view * shadowPos;
	//gl_Position.z -= 0.001;
}