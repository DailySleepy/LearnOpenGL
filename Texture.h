#pragma once
#include "TEXutility.h"
#include <optional>

struct TextureCore
{
	uint32_t id;
	uint64_t handle;

	TextureCore() = default;
	TextureCore(uint32_t width, uint32_t height, GLenum iFormat = GL_RGBA8, GLenum filterMode = GL_LINEAR);
	TextureCore(TextureCore&& other) noexcept;
	TextureCore& operator=(TextureCore&& other) noexcept;

	TextureCore(const TextureCore& other) = delete;
	TextureCore& operator=(const TextureCore& other) = delete;

	virtual ~TextureCore();
	void createHandle();
	virtual void bind(uint32_t shader_slot = 0);

	uint32_t getID() const;
	uint64_t getHandle() const;
};

struct TexTag
{
	int width = 0, height = 0;
	string type;
	string path;
	TexTag() = default;
	TexTag(string path) : path(path) {}
	TexTag(int width, int height) : width(width), height(height) {}
};

struct Texture : TextureCore
{
	std::optional<TexTag> tag;
	Texture() = default;
	Texture(uint32_t width, uint32_t height, GLenum iFormat = GL_RGBA8, GLenum filterMode = GL_LINEAR);
	Texture(std::string path, GLenum filterMode = GL_LINEAR);
	int getWidth() const;
	int getHeight() const;
	std::string getType() const;
	void setType(const std::string& t);
	std::string getPath() const;
	void setPath(const std::string& p);
};

struct ImageTexture : Texture
{
	uint64_t imageHandle;

	ImageTexture() = default;
	ImageTexture(std::string path, GLenum accessMode,
		GLenum accessFormat = GL_RGBA8, GLenum filterMode = GL_LINEAR);
	ImageTexture(uint32_t width, uint32_t height,
		GLenum accessMode, GLenum accessFormat = GL_RGBA8,
		GLenum iFormat = GL_RGBA8, GLenum filterMode = GL_LINEAR);
	ImageTexture(ImageTexture&& other) noexcept;
	ImageTexture& operator=(ImageTexture&& other) noexcept;
	~ImageTexture();

	void createImageHandle(GLenum accessMode, GLenum accessFormat = GL_RGBA8);
	uint64_t getImageHandle() const;
	void bindImage(uint32_t binding, GLenum access, GLenum accessFormat = GL_RGBA8);
};

struct Cubemap : TextureCore
{
	Cubemap() = default;
	Cubemap(uint32_t width, uint32_t height, GLenum iFormat = GL_RGBA8);
	Cubemap(std::vector<std::string> faces);

	void bind(uint32_t shader_slot = 0) override;
};

struct MultisampleTexture : TextureCore
{
	int samples;

	MultisampleTexture();
	MultisampleTexture(uint32_t width, uint32_t height, GLenum iFormat = GL_RGBA8, int samples = 4);
	void bind(uint32_t shader_slot = 0) override;
};