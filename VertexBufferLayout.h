#pragma once

#include <vector>
#include "GLCall.h"

struct VertexBufferElement
{
	unsigned int type;
	unsigned int count;
	unsigned int normalized;

	static unsigned int getSizeOfType(unsigned int type)
	{
		switch (type)
		{
			case GL_FLOAT: return 4;
			case GL_UNSIGNED_INT: return 4;
			case GL_UNSIGNED_BYTE: return 1;
		}
		ASSERT(false);
		return 0;
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_Elements;
	unsigned int m_stride;

public:
	VertexBufferLayout() : m_stride(0) {};

	template<typename T>
	void push(unsigned int count) = delete;

	template<>
	void push<float>(unsigned int count)
	{
		m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
		m_stride += sizeof(GLfloat) * count;
	}
	template<>
	void push<unsigned int>(unsigned int count)
	{
		m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_stride += sizeof(GLuint) * count;
	}
	template<>
	void push<unsigned char>(unsigned int count)
	{
		m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_stride += sizeof(GLubyte) * count;
	}

	inline const std::vector<VertexBufferElement>& getElements() const { return m_Elements; }

	inline unsigned int getStride() { return m_stride; }
};