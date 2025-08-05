#pragma once
#include "TEXutility.h"

struct TextureCore
{
	uint32_t id;
	uint64_t handle;

	TextureCore() : id(0), handle(0) {};
	TextureCore(uint32_t width, uint32_t height, GLenum iFormat = GL_RGBA8)
	{
		id = TEX::createTexture(width, height, iFormat);
		createHandle();
	}
	virtual ~TextureCore()
	{
		if (handle) TEX::destroyTextureHandle(handle);
		if (id) TEX::destroyTexture(id);
	}
	void createHandle()
	{
		handle = TEX::createTextureHandle(id);
	}
	virtual void bind(uint32_t shader_slot = 0)
	{
		GLCall(glActiveTexture(GL_TEXTURE0 + shader_slot));
		GLCall(glBindTexture(GL_TEXTURE_2D, id));
	}

	uint32_t getID() const { return id; }
	uint64_t getHandle() const { return handle; }
};

struct TexTag
{
	int width, height;
	string type;
	string path;
	TexTag() : width(0), height(0) {}
	TexTag(int width, int height) : width(width), height(height) {}
};

struct Texture : TextureCore, TexTag
{
	Texture() = default;
	Texture(uint32_t width, uint32_t height, GLenum iFormat = GL_RGBA8) :
		TextureCore(width, height, iFormat), TexTag(width, height)
	{
	}
	Texture(string path)
	{
		id = TEX::loadTextureFromFilepath(path, &width, &height);
		createHandle();
	}
};

struct ImageTexture : Texture
{
	uint64_t imageHandle;

	ImageTexture() : Texture(), imageHandle(0) {}
	ImageTexture(string path) : Texture(path)
	{
		imageHandle = 0;
	}
	ImageTexture(uint32_t width, uint32_t height, GLenum iFormat = GL_RGBA8) :
		Texture(width, height, iFormat)
	{
		imageHandle = 0;
	}
	~ImageTexture()
	{
		if (imageHandle) TEX::destroyImageHandle(imageHandle);
	}

	void createImageHandle(GLenum accessFormat, GLenum accessMode)
	{
		imageHandle = TEX::createImageHandle(id, accessFormat, accessMode);
	}
	uint64_t getImageHandle() { return imageHandle; }
	void bindImage(uint32_t binding, GLenum access, GLenum accessFormat = GL_RGBA8)
	{
		GLCall(glBindImageTexture(binding, id, 0, GL_FALSE, 0, access, accessFormat));
	}
};

struct CubemapCore : TextureCore
{
	CubemapCore() = default;
	CubemapCore(uint32_t width, uint32_t height, GLenum iFormat = GL_RGBA8)
	{
		id = TEX::createCubemap(width, height, iFormat);
	}

	void bind(uint32_t shader_slot = 0) override
	{
		GLCall(glActiveTexture(GL_TEXTURE0 + shader_slot));
		GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
	}
};

struct Cubemap : CubemapCore, TexTag
{
	Cubemap(vector<string> faces)
	{
		id = TEX::loadCubemapFromFilepath(faces);
	}
};

struct MultisampleTextureCore : TextureCore
{
	int samples;

	MultisampleTextureCore() : TextureCore(), samples(4) {}
	MultisampleTextureCore(uint32_t width, uint32_t height, GLenum iFormat = GL_RGBA8, int samples = 4)
		: TextureCore(), samples(samples)
	{
		id = TEX::createMultisampleTexture(width, height, iFormat, samples);
	}
	void bind(uint32_t shader_slot = 0) override
	{
		glActiveTexture(GL_TEXTURE0 + shader_slot);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, id);
	}
};
