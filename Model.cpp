#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Model.h"

Model::Model(const string& path)
{
	loadModel(path);
}

void Model::draw(Shader& shader, int count)
{
	for (auto& mesh : meshes)
		mesh->draw(shader, count);
}

void Model::addInstanceData(const void* data, int size, VertexBufferLayout& layout)
{
	ivbo.emplace_back(make_unique<VertexBuffer>(data, size));
	for (auto& mesh : meshes)
	{
		unsigned int vao = mesh->getVAO();
		VertexArray::addInstanceBuffer(vao, layout, VertexMemberCountOfThisModel);
	}
	VertexMemberCountOfThisModel++;
}

void Model::loadModel(const string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_Triangulate | aiProcess_GenSmoothNormals
		//| aiProcess_CalcTangentSpace
	);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR: Assimp: " << importer.GetErrorString() << endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

void Model::processNode(const aiNode* node, const aiScene* scene)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.emplace_back(processMesh(mesh, scene));
	}
	for (int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

void Model::computeTangentsAndBitangents(const aiMesh* mesh, std::vector<Vertex>& vertices)
{
	// 初始化切向量和半切向量为零
	for (Vertex& vertex : vertices)
	{
		vertex.tangent = glm::vec3(0.0f);
		vertex.bitangent = glm::vec3(0.0f);
	}

	// 遍历每个三角形
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace& face = mesh->mFaces[i];
		if (face.mNumIndices != 3) continue; // 仅处理三角形

		// 获取三个顶点的索引
		unsigned int i0 = face.mIndices[0];
		unsigned int i1 = face.mIndices[1];
		unsigned int i2 = face.mIndices[2];

		// 获取顶点位置
		glm::vec3 p0(mesh->mVertices[i0].x, mesh->mVertices[i0].y, mesh->mVertices[i0].z);
		glm::vec3 p1(mesh->mVertices[i1].x, mesh->mVertices[i1].y, mesh->mVertices[i1].z);
		glm::vec3 p2(mesh->mVertices[i2].x, mesh->mVertices[i2].y, mesh->mVertices[i2].z);

		// 获取 UV 坐标
		glm::vec2 uv0, uv1, uv2;
		if (mesh->mTextureCoords[0])
		{
			uv0 = glm::vec2(mesh->mTextureCoords[0][i0].x, mesh->mTextureCoords[0][i0].y);
			uv1 = glm::vec2(mesh->mTextureCoords[0][i1].x, mesh->mTextureCoords[0][i1].y);
			uv2 = glm::vec2(mesh->mTextureCoords[0][i2].x, mesh->mTextureCoords[0][i2].y);
		}
		else
		{
			uv0 = uv1 = uv2 = glm::vec2(0.0f);
		}

		// 计算边向量
		glm::vec3 edge1 = p1 - p0;
		glm::vec3 edge2 = p2 - p0;

		// 计算 UV 差
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		// 计算切向量和半切向量
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y + 0.0001f); // 防止除零
		glm::vec3 tangent;
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		glm::vec3 bitangent;
		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		// 累加到顶点
		vertices[i0].tangent += tangent;
		vertices[i1].tangent += tangent;
		vertices[i2].tangent += tangent;
		vertices[i0].bitangent += bitangent;
		vertices[i1].bitangent += bitangent;
		vertices[i2].bitangent += bitangent;
	}

	// 正交化和归一化
	for (Vertex& vertex : vertices)
	{
		// 确保 T 和 N 正交
		vertex.tangent = glm::normalize(vertex.tangent - vertex.normal * glm::dot(vertex.normal, vertex.tangent));
		// 确保 B 和 N、T 正交
		vertex.bitangent = glm::normalize(vertex.bitangent - vertex.normal * glm::dot(vertex.normal, vertex.bitangent) - vertex.tangent * glm::dot(vertex.tangent, vertex.bitangent));
		// 归一化 N
		vertex.normal = glm::normalize(vertex.normal);
	}
}

unique_ptr<const Mesh> Model::processMesh(const aiMesh* mesh, const aiScene* scene)
{
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<shared_ptr<Texture>> textures;

	// vertex
	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		vec3 vec;

		vec.x = mesh->mVertices[i].x;
		vec.y = mesh->mVertices[i].y;
		vec.z = mesh->mVertices[i].z;
		vertex.pos = vec;

		if (mesh->HasNormals())
		{
			vec.x = mesh->mNormals[i].x;
			vec.y = mesh->mNormals[i].y;
			vec.z = mesh->mNormals[i].z;
			vertex.normal = vec;
		}

		if (mesh->mTextureCoords[0])
		{
			vec2 uv;
			uv.x = mesh->mTextureCoords[0][i].x;
			uv.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoord = uv;
		}
		else vertex.texCoord = vec2(0, 0);

		if (mesh->HasTangentsAndBitangents())
		{
			vec.x = mesh->mTangents[i].x;
			vec.y = mesh->mTangents[i].y;
			vec.z = mesh->mTangents[i].z;
			vertex.tangent = vec;

			vec.x = mesh->mBitangents[i].x;
			vec.y = mesh->mBitangents[i].y;
			vec.z = mesh->mBitangents[i].z;
			vertex.bitangent = vec;
		}
		else
		{
			vertex.tangent = vec3(1);
			vertex.bitangent = vec3(0);
		}

		vertices.push_back(vertex);
	}
	//computeTangentsAndBitangents(mesh, vertices);

	// indices
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

		//TEX::printAllTextureType(mat);

		if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			vector<shared_ptr<Texture>> diffuseMaps = loadTexture(scene, mat, aiTextureType_DIFFUSE, "diffuse");
			textures.insert(textures.end(),
				std::make_move_iterator(diffuseMaps.begin()),
				std::make_move_iterator(diffuseMaps.end()));
		}
		if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0)
		{
			vector<shared_ptr<Texture>> specularMaps = loadTexture(scene, mat, aiTextureType_SPECULAR, "specular");
			textures.insert(textures.end(),
				std::make_move_iterator(specularMaps.begin()),
				std::make_move_iterator(specularMaps.end()));
		}
		if (mat->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			vector<shared_ptr<Texture>> normalMaps = loadTexture(scene, mat, aiTextureType_NORMALS, "normal");
			textures.insert(textures.end(),
				std::make_move_iterator(normalMaps.begin()),
				std::make_move_iterator(normalMaps.end()));
		}
		if (mat->GetTextureCount(aiTextureType_HEIGHT) > 0)
		{
			vector<shared_ptr<Texture>> heightMaps = loadTexture(scene, mat, aiTextureType_HEIGHT, "height");
			textures.insert(textures.end(),
				std::make_move_iterator(heightMaps.begin()),
				std::make_move_iterator(heightMaps.end()));
		}
	}

	return make_unique<const Mesh>(vertices, indices, textures);
}

vector<shared_ptr<Texture>> Model::loadTexture(const aiScene* scene, aiMaterial* mat, aiTextureType type, string typeName)
{
	vector<shared_ptr<Texture>> textures;
	for (int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		const char* fileName = str.C_Str();

		bool skip = false;
		for (int j = 0; j < loaded_textures.size(); j++)
		{
			if (strcmp(loaded_textures[j]->path.c_str(), fileName) == 0)
			{
				textures.push_back(loaded_textures[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			shared_ptr<Texture> texture = make_shared<Texture>();
			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(fileName);

			if (embeddedTexture != nullptr)
				texture->id = TEX::loadTextureFromModel(embeddedTexture);
			else
				texture->id = TEX::loadTextureFromFilepath(directory + '/' + fileName);
			texture->handle = TEX::createTextureHandle(texture->id);
			texture->type = typeName;
			texture->path = fileName;
			textures.push_back(texture);
			loaded_textures.push_back(texture);
		}
	}
	return textures;
}
