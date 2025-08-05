#pragma once
#include <vector>
#include "GLCall.h"
#include "Texture.h"

enum class FrameBufferType
{
	COLOR, COLOR_AND_RENDER, COLOR_FLOAT, COLOR_FLOAT_AND_RENDER,
	DEPTH, DEPTH_CUBEMAP, MULTISAMPLE, VELOCITY, G_BUFFER
};

class FrameBuffer
{
private:
	int width, height;

	unsigned int m_RendererID;

	vector<unique_ptr<TextureCore>> m_ColorTexture;
	unique_ptr<TextureCore> m_DepthTexture;
	unique_ptr<CubemapCore> m_DepthCubemap;
	unique_ptr<MultisampleTextureCore> m_ColorMultisampleTexture;
	unsigned int m_RenderBuffer = 0;
	unsigned int m_RenderBufferMultisample = 0;
public:
	FrameBuffer() = delete;
	FrameBuffer(FrameBuffer&) = delete;
	FrameBuffer(int width, int height);
	FrameBuffer(int width, int height, FrameBufferType type, int colorTexCount = 1);
	FrameBuffer(int width, int height, bool isDepth, bool isMultisample) = delete;
	~FrameBuffer();

	void attachColorTexture(unsigned int index = 0, GLenum iFormat = GL_RGB8);
	void attachDepthTexture(GLenum iFormat = GL_DEPTH_COMPONENT32F);
	void attachDepthCubemap(GLenum iFormat = GL_DEPTH_COMPONENT32F);
	void attachColorTextureMultisample(GLenum iFormat = GL_RGB8, unsigned int samples = 4);
	void attachRenderBuffer();
	void attachRenderBufferMultisample(unsigned int samples = 4);
	void check();

	void blitColorFrom(FrameBuffer& readFBO, int readAttachmentIndex = 0, int drawAttachmentIndex = 0);
	static void blit(GLenum bitType,
		unsigned int readFBO, int readWidth, int readHeight,
		unsigned int drawFBO, int drawWidth, int drawHeight,
		int readAttachmentIndex = 0, int drawAttachmentIndex = 0);

	void bind();
	void unbind();
	static void bindDefault();

	void bindDepthTex(unsigned int shader_slot = 0);
	void bindDepthCubemap(unsigned int shader_slot = 0);
	void bindColorTex(unsigned int shader_slot = 0, unsigned int texture_index = 0);
	void bindColorTexMultisample(unsigned int slot = 0);

	unsigned int getFBO() { return m_RendererID; }
	int getWidth() { return width; }
	int getHeight() { return height; }

	uint64_t getColorTextureHandle(unsigned int index = 0) { return m_ColorTexture[index]->handle; }
	uint64_t getDepthTextureHandle() { return m_DepthTexture->handle; }
	uint64_t getDepthCubemaphandle() { return m_DepthCubemap->handle; }
	uint64_t getColorTextureMultisampleHandle() { return m_ColorMultisampleTexture->id; }
};

