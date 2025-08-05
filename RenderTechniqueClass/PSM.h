#pragma once
#include "GLCall.h"
#include "FrameBuffer.h"
#include "UniformBuffer.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
using namespace glm;

class PSM
{
private:
	const vec3 dirs[6] = {
		vec3(1.0f,  0.0f,  0.0f),  // +X
		vec3(-1.0f, 0.0f,  0.0f),  // -X
		vec3(0.0f,  1.0f,  0.0f),  // +Y
		vec3(0.0f, -1.0f,  0.0f),  // -Y
		vec3(0.0f,  0.0f,  1.0f),  // +Z
		vec3(0.0f,  0.0f, -1.0f)   // -Z
	};
	const vec3 ups[6] = {
		-vec3(0.0f, 1.0f,  0.0f),  // +X, 使用 -Y 作为 up
		-vec3(0.0f, 1.0f,  0.0f),  // -X, 使用 -Y 作为 up
		 vec3(0.0f, 0.0f,  1.0f),  // +Y, 使用 +Z 作为 up
		 vec3(0.0f, 0.0f, -1.0f),  // -Y, 使用 -Z 作为 up
		-vec3(0.0f, 1.0f,  0.0f),  // +Z, 使用 -Y 作为 up
		-vec3(0.0f, 1.0f,  0.0f)   // -Z, 使用 -Y 作为 up
	};

public:
	FrameBuffer fbo;

	int width, height;
	float near, far;
	mat4 lightPV[6];

	PSM(int w, int h, int n, int f) : width(w), height(h), near(n), far(f), fbo(w, h, FrameBufferType::DEPTH_CUBEMAP) {}

	void updateLight(vec3 lightPos)
	{
		float aspect = float(width) / height;
		mat4 projection = perspective(radians(90.0f), aspect, near, far);
		for (int i = 0; i < 6; i++)
			lightPV[i] = projection * lookAt(lightPos, lightPos + dirs[i], ups[i]);
	}
};