#include "GLCall.h"
#include "VertexArray.h";

VertexArray::VertexArray(int index) : index(index)
{
	GLCall(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray()
{
	GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::addBuffer(VertexBufferLayout& layout)
{
	bind();
	const auto& elements = layout.getElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		GLCall(glEnableVertexAttribArray(i));
		GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), (const void*)offset));
		offset += element.count * VertexBufferElement::getSizeOfType(element.type);
	}
}

void VertexArray::addInstanceBuffer(VertexBufferLayout& layout, int index)
{
	bind();
	const auto& elements = layout.getElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		GLCall(glEnableVertexAttribArray(index + i));
		GLCall(glVertexAttribPointer(index + i, element.count, element.type, element.normalized, layout.getStride(), (const void*)offset));
		offset += element.count * VertexBufferElement::getSizeOfType(element.type);
		glVertexAttribDivisor(index + i, 1);
	}
}

void VertexArray::addInstanceBuffer(unsigned int vao, VertexBufferLayout& layout, int index)
{
	GLCall(glBindVertexArray(vao));
	const auto& elements = layout.getElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		GLCall(glEnableVertexAttribArray(index + i));
		GLCall(glVertexAttribPointer(index + i, element.count, element.type, element.normalized, layout.getStride(), (const void*)offset));
		offset += element.count * VertexBufferElement::getSizeOfType(element.type);
		glVertexAttribDivisor(index + i, 1);
	}
}

void VertexArray::bind() const
{
	GLCall(glBindVertexArray(m_RendererID));

	if (!glIsVertexArray(m_RendererID))
		cout << "ERROR: VAO is invalid, ID: " << m_RendererID << endl;
}

void VertexArray::unbind() const
{
	GLCall(glBindVertexArray(0));
}
