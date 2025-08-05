#include "GLCall.h"
#include "UniformBuffer.h"
#include "Shader.h"

UniformBuffer::UniformBuffer(unsigned int size)
{
	GLCall(glGenBuffers(1, &m_RendererID));
	bind();
	GLCall(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW));
	unbind();
}

UniformBuffer::UniformBuffer(unsigned int size, const void* data)
{
	GLCall(glGenBuffers(1, &m_RendererID));
	bind();
	GLCall(glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW));
	unbind();
}

UniformBuffer::~UniformBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void UniformBuffer::bind()
{
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID));
}

void UniformBuffer::unbind()
{
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void UniformBuffer::bindBlock(Shader& shader, string& blockName, unsigned int binding)
{
	unsigned int blockIndex = glGetUniformBlockIndex(shader.getShaderID(), blockName.c_str());
	if (blockIndex == GL_INVALID_INDEX)
	{
		std::cerr << "Error: Uniform block '" << blockName << "' not found in shader program." << std::endl;
	}
	GLCall(glUniformBlockBinding(shader.getShaderID(), blockIndex, binding));
	GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID));
}

void UniformBuffer::setSubData(unsigned int offset, unsigned int size, const void* data)
{
	bind();
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
	unbind();
}
