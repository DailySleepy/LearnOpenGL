#pragma once

class VertexBuffer
{
private:
	unsigned int m_RendererID = 0;
public:
	VertexBuffer() = default;
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void genBuffer(const void* data, unsigned int size);

	void bind() const;
	void unbind() const;

	unsigned int getVBO() { return m_RendererID; }
};
