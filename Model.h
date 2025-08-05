#pragma once
#include "Mesh.h"

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

class Model
{
private:
	vector<shared_ptr<Texture>> loaded_textures;
	vector<unique_ptr<const Mesh>> meshes;
	string directory;

	vector<unique_ptr<VertexBuffer>> ivbo;
	int VertexMemberCountOfThisModel = VertexMemberCount;

public:
	Model(const string& path);
	void draw(Shader& shader, int count = 1);
	void addInstanceData(const void* data, int size, VertexBufferLayout& layout);

private:
	void loadModel(const string& path);
	void processNode(const aiNode* node, const aiScene* scene);
	unique_ptr<const Mesh> processMesh(const aiMesh* mesh, const aiScene* scene);
	vector<shared_ptr<Texture>> loadTexture(const aiScene* scene, aiMaterial* mat, aiTextureType type, string typeName);

	void computeTangentsAndBitangents(const aiMesh* mesh, std::vector<Vertex>& vertices);
};
