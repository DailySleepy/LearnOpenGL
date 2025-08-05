#pragma once
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Renderer.h"
#include "Texture.h"
#include "shader.h"

using namespace glm;

const int VertexMemberCount = 5;

struct Vertex
{
	vec3 pos;
	vec3 normal;
	vec2 texCoord;
	vec3 tangent;
	vec3 bitangent;
};

class Mesh
{
private:
	VertexBufferLayout layout;
	VertexArray vao;
	VertexBuffer vbo;
	IndexBuffer ibo;

public:
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<shared_ptr<Texture>> textures;

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<shared_ptr<Texture>> textures);
	void draw(Shader& shader, int count = 1) const;
	unsigned int getVAO() const { return vao.getVAO(); }
};