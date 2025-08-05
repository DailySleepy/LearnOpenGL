#pragma once

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class VertexArray
{
private:
	unsigned int m_RendererID;
	unsigned int index;
public:
	VertexArray(int index = 0);
	~VertexArray();

	void addBuffer(VertexBufferLayout& layout);
	void addInstanceBuffer(VertexBufferLayout& layout, int index);
	static void addInstanceBuffer(unsigned int vao, VertexBufferLayout& layout, int index);
	void bind() const;
	void unbind() const;
	unsigned int getVAO() const { return m_RendererID; }
};