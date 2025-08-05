#pragma once
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"

class ScreenQuad
{
private:
	const float vertices[16] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,  // ���Ͻ�
		-1.0f, -1.0f,  0.0f, 0.0f,  // ���½�
		 1.0f, -1.0f,  1.0f, 0.0f,  // ���½�
		 1.0f,  1.0f,  1.0f, 1.0f   // ���Ͻ�
	};
	float pixelToNDC_X(float pixel_x)
	{
		return -1.0f + 2.0f * pixel_x / window_width;
	}
	float pixelToNDC_Y(float pixel_y)
	{
		return -1.0f + 2.0f * pixel_y / window_height;
	}
	vector<float> calculateVertices1()
	{
		vector<float> vertices1 = {
			// positions              // texCoords
			pixelToNDC_X(0.0f),       pixelToNDC_Y(main_size),  0.0f, 1.0f, // ���Ͻ�
			pixelToNDC_X(0.0f),       pixelToNDC_Y(0.0f),       0.0f, 0.0f, // ���½�
			pixelToNDC_X(main_size),  pixelToNDC_Y(0.0f),       1.0f, 0.0f, // ���½�
			pixelToNDC_X(main_size),  pixelToNDC_Y(main_size),  1.0f, 1.0f  // ���Ͻ�
		};
		return vertices1;
	}
	vector<float> calculateVertices2()
	{
		vector<float> vertices2 = {
			// positions                        // texCoords
			pixelToNDC_X(main_size),           pixelToNDC_Y(sub_size),  0.0f, 1.0f, // ���Ͻ�
			pixelToNDC_X(main_size),           pixelToNDC_Y(0.0f),      0.0f, 0.0f, // ���½�
			pixelToNDC_X(main_size + sub_size), pixelToNDC_Y(0.0f),      1.0f, 0.0f, // ���½�
			pixelToNDC_X(main_size + sub_size), pixelToNDC_Y(sub_size),  1.0f, 1.0f  // ���Ͻ�
		};
		return vertices2;
	}

	const unsigned int indices[6] = {
		0, 1, 2,
		2, 3, 0
	};

	VertexBufferLayout layout;
	VertexArray vao;
	VertexBuffer vbo;
	IndexBuffer ibo;

	int main_size = 0, sub_size = 0, window_width = 0, window_height = 0;

public:
	ScreenQuad() : vbo(vertices, sizeof(vertices)), ibo(indices, sizeof(indices) / sizeof(indices[0]))
	{
		layout.push<float>(2);
		layout.push<float>(2);
		vao.addBuffer(layout);
	}

	ScreenQuad(int area, int main_size, int sub_size) :
		ibo(indices, sizeof(indices) / sizeof(indices[0])),
		main_size(main_size), sub_size(sub_size), window_width(main_size + sub_size), window_height(main_size)
	{
		vector<float> vertices;
		if (area == 1) vertices = calculateVertices1();
		else if (area == 2) vertices = calculateVertices2();
		vbo.genBuffer(vertices.data(), vertices.size() * sizeof(float));

		layout.push<float>(2);
		layout.push<float>(2);
		vao.addBuffer(layout);
	}

	void draw(const Shader& shader)
	{
		vao.bind();
		vbo.bind();
		ibo.bind();
		shader.bind();
		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
	}
};