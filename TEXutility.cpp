#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"
#include "stb_image_write.h"
#include "TEXutility.h"

GLenum getFormat(GLenum iFormat)
{
	switch (iFormat)
	{
		case GL_R8:
		case GL_R16:
		case GL_R16F:
		case GL_R32F:
			return GL_RED;

		case GL_RG8:
		case GL_RG16:
		case GL_RG16F:
		case GL_RG32F:
			return GL_RG;

		case GL_RGB8:
		case GL_SRGB8:
		case GL_RGB16:
		case GL_RGB16F:
		case GL_RGB32F:
			return GL_RGB;

		case GL_RGBA8:
		case GL_SRGB8_ALPHA8:
		case GL_RGBA16:
		case GL_RGBA16F:
		case GL_RGBA32F:
			return GL_RGBA;

		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32:
			return GL_DEPTH_COMPONENT;

		default:
			std::cerr << "getFormat(): Unsupported iFormat: " << iFormat << std::endl;
			return GL_NONE;
	}
}
GLenum getType(GLenum iFormat)
{
	switch (iFormat)
	{
		case GL_RGB8:
		case GL_RGBA8:
		case GL_SRGB8:
		case GL_SRGB8_ALPHA8:
			return GL_UNSIGNED_BYTE;

		case GL_RGB16:
		case GL_RGBA16:
			return GL_UNSIGNED_SHORT;

		case GL_RGB16F:
		case GL_RGBA16F:
		case GL_RG16F:
		case GL_R16F:
		case GL_DEPTH_COMPONENT16:
			return GL_HALF_FLOAT;

		case GL_RGB32F:
		case GL_RGBA32F:
		case GL_RG32F:
		case GL_R32F:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32:
			return GL_FLOAT;

		default:
			std::cerr << "getType(): Unsupported iFormat: " << iFormat << std::endl;
			return GL_NONE;
	}
}

void TEX::setTexFilter(GLenum minFilterMode, GLenum magFilterMode)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterMode);
}

void TEX::setTexWarp(GLenum wrapMode, int dimension)
{
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
	if (dimension == 3)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);
	}
	else if (dimension <= 1 || dimension >= 4)
	{
		cerr << "dimension = " << dimension << " is not supported" << endl;
	}
}

uint32_t TEX::createTexture(int width, int height, GLenum iFormat, GLenum filterMode)
{
	uint32_t textureID;
	GLCall(glGenTextures(1, &textureID));
	GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, iFormat, width, height, 0, getFormat(iFormat), getType(iFormat), nullptr));
	setTexFilter(filterMode, filterMode);
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

uint32_t TEX::createMultisampleTexture(int width, int height, GLenum iFormat, int samples)
{
	uint32_t textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, iFormat, width, height, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

uint32_t TEX::createDepthTexture(int width, int height, GLenum iFormat)
{
	uint32_t textureID;
	GLCall(glGenTextures(1, &textureID));
	GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, iFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
	setTexFilter(GL_NEAREST, GL_NEAREST);
	setTexWarp(GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1, 1, 1, 1 };
	GLCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

uint32_t TEX::createCubemap(int width, int height, GLenum iFormat)
{
	uint32_t textureID;
	GLCall(glGenTextures(1, &textureID));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

	for (int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, iFormat,
			width, height, 0, getFormat(iFormat), getType(iFormat), NULL);

	setTexFilter(GL_LINEAR, GL_LINEAR);
	setTexWarp(GL_CLAMP_TO_EDGE, 3);

	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

uint32_t TEX::createDepthCubemap(int width, int height, GLenum iFormat)
{
	uint32_t textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, iFormat,
			width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	setTexFilter(GL_NEAREST, GL_NEAREST);
	setTexWarp(GL_CLAMP_TO_BORDER, 3);
	float borderColor[] = { 1, 1, 1, 1 };
	glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

uint64_t TEX::createTextureHandle(uint32_t textureID)
{
	uint64_t handle;
	GLCall(handle = glGetTextureHandleARB(textureID));
	GLCall(glMakeTextureHandleResidentARB(handle));
	return handle;
}

uint64_t TEX::createImageHandle(uint32_t textureID, GLenum accessMode, GLenum accessFormat)
{
	uint64_t handle;
	GLCall(handle = glGetImageHandleARB(textureID, 0, GL_FALSE, 0, accessFormat));
	GLCall(glMakeImageHandleResidentARB(handle, accessMode));
	return handle;
}

void TEX::checkTextureHandle(uint64_t handle)
{
	if (!glIsTextureHandleResidentARB(handle))
	{
		cerr << "Texture handle is not resident!" << endl;
	}
	else cout << "Texture handle is resident" << endl;
}

void TEX::checkImageHandle(uint64_t handle)
{
	if (!glIsImageHandleResidentARB(handle))
	{
		cerr << "Image handle is not resident!" << endl;
	}
	else cout << "Image handle is resident" << endl;
}

uint32_t TEX::loadTextureFromFilepath(string path)
{
	stbi_set_flip_vertically_on_load(true);

	uint32_t textureID;
	GLCall(glGenTextures(1, &textureID));
	GLCall(glBindTexture(GL_TEXTURE_2D, textureID));

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum iFormat = GL_RGB8;
		switch (nrChannels)
		{
			case 1: iFormat = GL_RED; break;
			case 3: iFormat = GL_RGB8; break;
			case 4: iFormat = GL_RGBA8; break;
		}
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, iFormat, width, height, 0, getFormat(iFormat), GL_UNSIGNED_BYTE, data));
		setTexWarp();
		setTexFilter();
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	}
	else
		std::cerr << "Failed to load texture from filepath: " << path << " - " << stbi_failure_reason() << std::endl;

	stbi_image_free(data);

	return textureID;
}

uint32_t TEX::loadTextureFromFilepath(string path, int* w, int* h, GLenum filterMode)
{
	if (w == nullptr || h == nullptr)
	{
		cerr << "pointer should not be null" << endl;
		throw runtime_error("Accessing a null pointer");
	}

	stbi_set_flip_vertically_on_load(true);

	uint32_t textureID;
	GLCall(glGenTextures(1, &textureID));
	GLCall(glBindTexture(GL_TEXTURE_2D, textureID));

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4); //
	if (data)
	{
		*w = width;
		*h = height;
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
		// 强制4通道RGBA, 因为compute shader只接受可读写(imageLoad/imageStore)类型(rgba/rg/r), 其他shader支持rgb(使用texture()采样)
		setTexWarp();
		setTexFilter(filterMode, filterMode);
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	}
	else
		std::cerr << "Failed to load texture from filepath: " << path << " - " << stbi_failure_reason() << std::endl;

	stbi_image_free(data);

	return textureID;
}

uint32_t TEX::loadTextureFromModel(const aiTexture* aiTex)
{
	if (aiTex == nullptr) return 0;

	stbi_set_flip_vertically_on_load(true);

	uint32_t textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	int width, height, nrChannels;
	unsigned char* image_data = nullptr;
	int len;
	if (aiTex->mHeight == 0) len = aiTex->mWidth;
	else len = aiTex->mWidth * aiTex->mHeight;

	image_data = stbi_load_from_memory(
		reinterpret_cast<unsigned char*>(aiTex->pcData), len, &width, &height, &nrChannels, 0);

	if (image_data != nullptr)
	{
		GLenum iFormat = GL_RGB8;
		switch (nrChannels)
		{
			case 1: iFormat = GL_R8; break;
			case 3: iFormat = GL_RGB8; break;
			case 4: iFormat = GL_RGBA8; break;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, iFormat, width, height, 0, getFormat(iFormat), GL_UNSIGNED_BYTE, image_data);
		setTexWarp();
		setTexFilter();
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cerr << "Failed to load texture from model - " << stbi_failure_reason() << std::endl;

	return textureID;
}

uint32_t TEX::loadCubemapFromFilepath(vector<string> faces)
{
	stbi_set_flip_vertically_on_load(false);

	uint32_t textureID;
	GLCall(glGenTextures(1, &textureID));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

	int width, height, nrChannels;
	for (int i = 0; i < faces.size(); i++)
	{
		string path = faces[i];
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
		}
		else
		{
			std::cerr << "Failed to load cubemap from filepath: " << path << " - " << stbi_failure_reason() << std::endl;
		}
		stbi_image_free(data);
	}
	setTexWarp(GL_CLAMP_TO_EDGE, 3);
	setTexFilter(GL_LINEAR, GL_LINEAR);

	return textureID;
}

void TEX::saveTextureToJPG(GLuint textureID, int width, int height, const std::string& outputPath)
{
	stbi_flip_vertically_on_write(true);

	unsigned char* data = new unsigned char[width * height * 4];
	glBindTexture(GL_TEXTURE_2D, textureID);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	if (data && stbi_write_jpg(outputPath.c_str(), width, height, 4, data, 100))
		cout << "Image saved to " << outputPath << endl;
	else
		cout << "Fail to save image to " << outputPath << endl;

	delete[] data;
	glBindTexture(GL_TEXTURE_2D, 0);
}

string TEX::textureTypeToString(aiTextureType type)
{
	switch (type)
	{
		case aiTextureType_DIFFUSE:
			return "Diffuse";
		case aiTextureType_SPECULAR:
			return "Specular";
		case aiTextureType_AMBIENT:
			return "Ambient";
		case aiTextureType_EMISSIVE:
			return "Emissive";
		case aiTextureType_HEIGHT:
			return "Height";
		case aiTextureType_NORMALS:
			return "Normals";
		case aiTextureType_SHININESS:
			return "Shininess";
		case aiTextureType_OPACITY:
			return "Opacity";
		case aiTextureType_DISPLACEMENT:
			return "Displacement";
		case aiTextureType_LIGHTMAP:
			return "Lightmap";
		case aiTextureType_REFLECTION:
			return "Reflection";
		case aiTextureType_BASE_COLOR:
			return "Base Color";
		case aiTextureType_METALNESS:
			return "Metalness";
		case aiTextureType_DIFFUSE_ROUGHNESS:
			return "Diffuse Roughness";
		case aiTextureType_AMBIENT_OCCLUSION:
			return "Ambient Occlusion";
		case aiTextureType_UNKNOWN:
		default:
			return "Unknown";
	}
}

void TEX::printAllTextureType(aiMaterial* mat)
{
	aiString path;

	for (int tt = aiTextureType_NONE; tt <= aiTextureType_UNKNOWN; tt++)
	{
		aiTextureType type = static_cast<aiTextureType>(tt);
		unsigned int count = mat->GetTextureCount(type);
		if (count > 0)
		{
			std::cout << "Texture Type: " << textureTypeToString(type)
				<< " | Count: " << count << std::endl;

			for (unsigned int i = 0; i < count; i++)
			{
				if (AI_SUCCESS == mat->GetTexture(type, i, &path))
				{
					std::cout << "  - Path: " << path.C_Str() << std::endl;
				}
			}
		}
	}
}