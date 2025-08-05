#include "FrameBuffer.h"
#include "Texture.h"

FrameBuffer::FrameBuffer(int width, int height) : width(width), height(height)
{
	GLCall(glGenFramebuffers(1, &m_RendererID));
}

FrameBuffer::FrameBuffer(int width, int height, FrameBufferType type, int colorTexCount) : width(width), height(height)
{
	GLCall(glGenFramebuffers(1, &m_RendererID));

	bind();

	switch (type)
	{
		case FrameBufferType::COLOR:
			for (int i = 0; i < colorTexCount; i++)
				attachColorTexture(i);
			break;

		case FrameBufferType::COLOR_AND_RENDER:
			for (int i = 0; i < colorTexCount; i++)
				attachColorTexture(i);
			attachRenderBuffer();
			break;

		case FrameBufferType::COLOR_FLOAT:
			for (int i = 0; i < colorTexCount; i++)
				attachColorTexture(i, GL_RGB16F);
			break;

		case FrameBufferType::COLOR_FLOAT_AND_RENDER:
			for (int i = 0; i < colorTexCount; i++)
				attachColorTexture(i, GL_RGB16F);
			attachRenderBuffer();
			break;

		case FrameBufferType::DEPTH:
			attachDepthTexture();
			break;

		case FrameBufferType::DEPTH_CUBEMAP:
			attachDepthCubemap();
			break;

		case FrameBufferType::VELOCITY:
			attachColorTexture(0, GL_RG16F);
			attachRenderBuffer();
			break;

		case FrameBufferType::MULTISAMPLE:
			attachColorTextureMultisample();
			attachRenderBufferMultisample();
			break;

		case FrameBufferType::G_BUFFER:
			attachColorTexture(0, GL_RGB16F); // position
			attachColorTexture(1, GL_RGB16F); // normal
			attachColorTexture(2, GL_RGBA8);  // albedo and specular
			colorTexCount = 3;
			//attachRenderBuffer();
			attachDepthTexture(GL_DEPTH_COMPONENT24);
			break;
	}

	if (colorTexCount > 1)
	{
		vector<unsigned int> attachments(colorTexCount);
		for (int i = 0; i < colorTexCount; i++)
			attachments[i] = GL_COLOR_ATTACHMENT0 + i;
		GLCall(glDrawBuffers(colorTexCount, attachments.data()));
	}

	check();
	unbind();
}

FrameBuffer::~FrameBuffer()
{
	GLCall(glDeleteFramebuffers(1, &m_RendererID));
	if (m_RenderBuffer != 0) GLCall(glDeleteRenderbuffers(1, &m_RenderBuffer));
	if (m_RenderBufferMultisample != 0) GLCall(glDeleteRenderbuffers(1, &m_RenderBufferMultisample));
}

void FrameBuffer::attachColorTexture(unsigned int index, GLenum iFormat)
{
	m_ColorTexture.resize(index + 1);
	m_ColorTexture[index] = make_unique<TextureCore>(width, height, iFormat);
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, m_ColorTexture[index]->id, 0));
}

void FrameBuffer::attachDepthTexture(GLenum iFormat)
{
	m_DepthTexture = make_unique<TextureCore>();
	m_DepthTexture->id = TEX::createDepthTexture(width, height, iFormat);
	m_DepthTexture->createHandle();
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture->id, 0));
	GLCall(glDrawBuffer(GL_NONE));
	GLCall(glReadBuffer(GL_NONE));
}

void FrameBuffer::attachDepthCubemap(GLenum iFormat)
{
	m_DepthCubemap = make_unique<CubemapCore>();
	m_DepthCubemap->id = TEX::createDepthCubemap(width, height, iFormat);
	m_DepthCubemap->createHandle();

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemap->id, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
}

void FrameBuffer::attachColorTextureMultisample(GLenum iFormat, unsigned int samples)
{
	m_ColorMultisampleTexture = make_unique<MultisampleTextureCore>();
	m_ColorMultisampleTexture->id = TEX::createMultisampleTexture(width, height, iFormat, samples);
	m_ColorMultisampleTexture->createHandle();
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_ColorMultisampleTexture->id, 0));
}

void FrameBuffer::attachRenderBuffer()
{
	GLCall(glGenRenderbuffers(1, &m_RenderBuffer));
	GLCall(glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffer));
	GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
	GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffer));
}

void FrameBuffer::attachRenderBufferMultisample(unsigned int samples)
{
	GLCall(glGenRenderbuffers(1, &m_RenderBufferMultisample));
	GLCall(glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBufferMultisample));
	GLCall(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height));
	GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBufferMultisample));
}

void FrameBuffer::check()
{
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR: framebuffer is not complete, " << std::endl;
}

void FrameBuffer::blitColorFrom(FrameBuffer& readFBO, int readAttachmentIndex, int drawAttachmentIndex)
{
	GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO.getFBO()));
	GLCall(glReadBuffer(GL_COLOR_ATTACHMENT0 + readAttachmentIndex));
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RendererID));
	GLCall(glDrawBuffer(GL_COLOR_ATTACHMENT0 + drawAttachmentIndex));

	GLCall(glBlitFramebuffer(0, 0, readFBO.getWidth(), readFBO.getHeight(), 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR)); // 如果是渲染缓冲区附件只能用GL_NEAREST

	GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

void FrameBuffer::blit(GLenum bitType, unsigned int readFBO, int readWidth, int readHeight, unsigned int drawFBO, int drawWidth, int drawHeight, int readAttachmentIndex, int drawAttachmentIndex)
{
	GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO));
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO));

	GLenum  filterMode;
	if (bitType == GL_COLOR_BUFFER_BIT)
	{
		GLCall(glReadBuffer(GL_COLOR_ATTACHMENT0 + readAttachmentIndex));
		GLCall(glReadBuffer(GL_COLOR_ATTACHMENT0 + drawAttachmentIndex));
		filterMode = GL_LINEAR;
	}
	else filterMode = GL_NEAREST; // bitType = DEPTH

	GLCall(glBlitFramebuffer(0, 0, readWidth, readHeight, 0, 0, drawWidth, drawHeight, bitType, filterMode));

	GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

void FrameBuffer::bind()
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));
}

void FrameBuffer::unbind()
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::bindDefault()
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::bindColorTex(unsigned int slot, unsigned int index)
{
	m_ColorTexture[index]->bind(slot);
}

void FrameBuffer::bindColorTexMultisample(unsigned int slot)
{
	m_ColorMultisampleTexture->bind(slot);
}

void FrameBuffer::bindDepthTex(unsigned int slot)
{
	m_DepthTexture->bind(slot);
}

void FrameBuffer::bindDepthCubemap(unsigned int slot)
{
	m_DepthCubemap->bind(slot);
}
