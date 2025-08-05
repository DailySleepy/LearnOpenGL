#include "GLCall.h"
#include "StorageBuffer.h"

auto a = GL_DYNAMIC_STORAGE_BIT;

StorageBuffer::StorageBuffer(unsigned int size, unsigned int usage) : size(size)
{
	GLCall(glGenBuffers(1, &m_RendererID));
	bind();
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, usage));
	unbind();
}

StorageBuffer::StorageBuffer(unsigned int size, const void* data, unsigned int usage) : size(size)
{
	GLCall(glGenBuffers(1, &m_RendererID));
	bind();
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage));
	unbind();
}

StorageBuffer::~StorageBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void StorageBuffer::bind()
{
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID));
}

void StorageBuffer::unbind()
{
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

void StorageBuffer::setSubData(unsigned int offset, unsigned int size, const void* data)
{
	bind();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
	unbind();
}

void StorageBuffer::getSubData(unsigned int offset, unsigned int size, void* data)
{
	bind();
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
	unbind();
}

void StorageBuffer::getData(unsigned int size, void* data)
{
	getSubData(0, size, data);
}
