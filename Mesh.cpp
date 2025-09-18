#include "Mesh.h"

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<shared_ptr<Texture>> textures)
	:vertices(vertices), indices(indices), textures(move(textures)),
	vbo(vertices.data(), vertices.size() * sizeof(Vertex)), ibo(indices.data(), indices.size())
{
	layout.push<float>(3); //pos
	layout.push<float>(3); //normal
	layout.push<float>(2); //texCoord
	layout.push<float>(3); //tangent
	layout.push<float>(3); //bitangent

	vao.addBuffer(layout);
}

void Mesh::draw(Shader& shader, int count) const
{
	vao.bind();
	vbo.bind();
	ibo.bind();
	shader.bind();

	int diffuseNr = 1;
	int specularNr = 1;
	int normalNr = 1;
	int heightNr = 1;
	int reflectionNr = 1;
	int ambientNr = 1;

	for (int i = 0; i < textures.size(); i++)
	{
		string number;
		string name = textures[i]->type;
		if (name == "diffuse") number = std::to_string(diffuseNr++);
		else if (name == "specular") number = std::to_string(specularNr++);
		else if (name == "normal") number = std::to_string(normalNr++);
		else if (name == "height") number = std::to_string(heightNr++);
		else if (name == "reflection") number = std::to_string(reflectionNr++);
		else if (name == "ambient") number = std::to_string(ambientNr++);

		shader.set("material." + name + number, textures[i]->getHandle());

		/*GLCall(glActiveTexture(GL_TEXTURE0 + i));
		GLCall(glBindTexture(GL_TEXTURE_2D, textures[i]->id));
		shader.setInt1("material." + name + number, i);*/
	}

	if (count == 1)
	{
		GLCall(glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr));
	}
	else
	{
		GLCall(glDrawElementsInstanced(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr, count));
	}
}
