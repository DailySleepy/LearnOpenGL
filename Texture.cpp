#include "Texture.h"

TextureCore::TextureCore(uint32_t width, uint32_t height, GLenum iFormat, GLenum filterMode)
{
	id = TEX::createTexture(width, height, iFormat, filterMode);
	handle = TEX::createTextureHandle(id);
}

TextureCore::TextureCore(TextureCore&& other) noexcept
	: id(other.id), handle(other.handle)
{
	other.id = 0;
	other.handle = 0;
}

TextureCore& TextureCore::operator=(TextureCore&& other) noexcept
{
	if (this != &other)
	{
		if (handle) TEX::destroyTextureHandle(handle);
		if (id) TEX::destroyTexture(id);

		id = other.id;
		handle = other.handle;

		other.id = 0;
		other.handle = 0;
	}
	return *this;
}

TextureCore::~TextureCore()
{
	if (handle) TEX::destroyTextureHandle(handle);
	if (id) TEX::destroyTexture(id);
}

void TextureCore::createHandle()
{
	handle = TEX::createTextureHandle(id);
}

void TextureCore::bind(uint32_t shader_slot)
{
	GLCall(glActiveTexture(GL_TEXTURE0 + shader_slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, id));
}

uint32_t TextureCore::getID() const
{
	return id;
}

uint64_t TextureCore::getHandle() const
{
	return handle;
}

Texture::Texture(uint32_t width, uint32_t height, GLenum iFormat, GLenum filterMode)
	: TextureCore(width, height, iFormat, filterMode)
{
	tag = TexTag(width, height);
}

Texture::Texture(std::string path, GLenum filterMode)
{
	tag = TexTag(path);
	id = TEX::loadTextureFromFilepath(path, &tag->width, &tag->height, filterMode);
	handle = TEX::createTextureHandle(id);
}

int Texture::getWidth() const
{
	return tag ? tag->width : 0;
}

int Texture::getHeight() const
{
	return tag ? tag->height : 0;
}

std::string Texture::getType() const
{
	return tag ? tag->type : "";
}

void Texture::setType(const std::string& t)
{
	if (!tag) tag.emplace();
	tag->type = t;
}

std::string Texture::getPath() const
{
	return tag ? tag->path : "";
}

void Texture::setPath(const std::string& p)
{
	if (!tag) tag.emplace();
	tag->path = p;
}

ImageTexture::ImageTexture(std::string path, GLenum accessMode, GLenum accessFormat, GLenum filterMode)
	: Texture(path, filterMode)
{
	createImageHandle(accessMode, accessFormat);
}

ImageTexture::ImageTexture(uint32_t width, uint32_t height, GLenum accessMode, GLenum accessFormat, GLenum iFormat, GLenum filterMode)
	: Texture(width, height, iFormat, filterMode)
{
	createImageHandle(accessMode, accessFormat);
}

ImageTexture::ImageTexture(ImageTexture&& other) noexcept
	: Texture(std::move(other)), imageHandle(other.imageHandle)
{
	other.imageHandle = 0;
}

ImageTexture& ImageTexture::operator=(ImageTexture&& other) noexcept
{
	if (this != &other)
	{
		if (imageHandle) TEX::destroyImageHandle(imageHandle);

		Texture::operator=(std::move(other));

		imageHandle = other.imageHandle;
		other.imageHandle = 0;
	}
	return *this;
}

ImageTexture::~ImageTexture()
{
	if (imageHandle) TEX::destroyImageHandle(imageHandle);
}

void ImageTexture::createImageHandle(GLenum accessMode, GLenum accessFormat)
{
	imageHandle = TEX::createImageHandle(id, accessMode, accessFormat);
}

uint64_t ImageTexture::getImageHandle() const
{
	return imageHandle;
}

void ImageTexture::bindImage(uint32_t binding, GLenum access, GLenum accessFormat)
{
	GLCall(glBindImageTexture(binding, id, 0, GL_FALSE, 0, access, accessFormat));
}

Cubemap::Cubemap(uint32_t width, uint32_t height, GLenum iFormat)
{
	id = TEX::createCubemap(width, height, iFormat);
}

Cubemap::Cubemap(std::vector<std::string> faces)
{
	id = TEX::loadCubemapFromFilepath(faces);
}

void Cubemap::bind(uint32_t shader_slot)
{
	GLCall(glActiveTexture(GL_TEXTURE0 + shader_slot));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
}

MultisampleTexture::MultisampleTexture() : TextureCore(), samples(4) {}

MultisampleTexture::MultisampleTexture(uint32_t width, uint32_t height, GLenum iFormat, int samples)
	: TextureCore(), samples(samples)
{
	id = TEX::createMultisampleTexture(width, height, iFormat, samples);
}

void MultisampleTexture::bind(uint32_t shader_slot)
{
	glActiveTexture(GL_TEXTURE0 + shader_slot);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, id);
}