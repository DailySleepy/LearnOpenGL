#pragma once
#include <string>

class Shader;

class UniformBuffer
{
private:
	unsigned int m_RendererID;
public:
	UniformBuffer(UniformBuffer&) = delete;
	UniformBuffer(unsigned int size);
	UniformBuffer(unsigned int size, const void* data);
	~UniformBuffer();
	void bind();
	void unbind();
	void bindBlock(Shader& shader, unsigned int binding) = delete;
	void bindBlock(Shader& shader, std::string& blockName, unsigned int binding);
	void setSubData(unsigned int offset, unsigned int size, const void* data);
	unsigned int getUBO() { return m_RendererID; }
};
