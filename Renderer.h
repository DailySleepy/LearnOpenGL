#pragma once
#include "GLCall.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "ScreenQuad.h"

class Renderer
{
public:
	void setClearColor(float r, float g, float b, float a) { glClearColor(r, g, b, a); }
	void setClearColor(vec3 v, float a) { glClearColor(v.r, v.g, v.b, a); }

	void clearAllBit() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
	void clearColorBit() { glClear(GL_COLOR_BUFFER_BIT); }
	void clearDepthBit() { glClear(GL_DEPTH_BUFFER_BIT); }
	void clearStencilBit() { glClear(GL_STENCIL_BUFFER_BIT); }

	void e_ColorWrite() { glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); }
	void d_ColorWrite() { glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); }

	void e_DepthTest() { glEnable(GL_DEPTH_TEST); }
	void e_DepthTestGreater() { e_DepthTest(); glDepthFunc(GL_GREATER); }
	void e_DepthTestLEqual() { e_DepthTest(); glDepthFunc(GL_LEQUAL); }
	void e_DepthTestLess() { e_DepthTest(); glDepthFunc(GL_LESS); }
	void d_DepthTest() { glDisable(GL_DEPTH_TEST); }

	void e_DepthWrite() { glDepthMask(GL_TRUE); }
	void d_DepthWrite() { glDepthMask(GL_FALSE); }

	void e_CullFace() { glEnable(GL_CULL_FACE); }
	void e_CullFaceBack() { e_CullFace(); glCullFace(GL_BACK); }
	void e_CullFaceFront() { e_CullFace(); glCullFace(GL_FRONT); }
	void d_CullFace() { glDisable(GL_CULL_FACE); }

	void e_Blend() { glEnable(GL_BLEND); }
	void e_BlendAdd() { e_Blend(); glBlendFunc(GL_ONE, GL_ONE); }
	void e_BlendAlpha() { e_Blend(); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }
	void d_Blend() { glDisable(GL_BLEND); }

	void setViewport(int w, int h) { glViewport(0, 0, w, h); }

	void draw(const VertexArray& vao, const VertexBuffer& vbo, const IndexBuffer& ibo, const Shader& shader, int count)
	{
		vao.bind();
		vbo.bind();
		ibo.bind();
		shader.bind();

		if (count == 1)
		{
			GLCall(glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr));
		}
		else
		{
			GLCall(glDrawElementsInstanced(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr, count));
		}
	}
};
