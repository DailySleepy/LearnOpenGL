#pragma once
#include <string>
#define GL_DYNAMIC_COPY 0x88EA

class Shader;

class StorageBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int size;
public:
	StorageBuffer(unsigned int size, unsigned int usage = GL_DYNAMIC_COPY);
	StorageBuffer(unsigned int size, const void* data, unsigned int usage = GL_DYNAMIC_COPY);
	~StorageBuffer();
	void bind();
	void unbind();
	void bindBlock(Shader& shader, unsigned int binding) = delete;
	void bindBlock(Shader& shader, std::string& blockName, unsigned int binding) = delete;
	void setSubData(unsigned int offset, unsigned int size, const void* data);
	void getSubData(unsigned int offset, unsigned int size, void* data);
	void getData(unsigned int size, void* data);
	unsigned int getSSBO() { return m_RendererID; }
};

