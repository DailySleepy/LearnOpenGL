#pragma once
#include <vector>
#include <string>
#include "GLCall.h"

struct aiMaterial;
struct aiTexture;
enum aiTextureType;

struct TEX
{
	static void setTexFilter(GLenum minFilterMode = GL_LINEAR_MIPMAP_LINEAR, GLenum magFilterMode = GL_LINEAR);
	static void setTexWarp(GLenum wrapMode = GL_REPEAT, int dimension = 2);

	static uint32_t createTexture(int width, int height, GLenum iFormat = GL_RGBA8);
	static uint32_t createMultisampleTexture(int width, int height, GLenum iFormat = GL_RGBA8, int samples = 4);
	static uint32_t createDepthTexture(int width, int height, GLenum iFormat = GL_DEPTH_COMPONENT32);
	static uint32_t createCubemap(int width, int height, GLenum iFormat = GL_RGBA8);
	static uint32_t createDepthCubemap(int width, int height, GLenum iFormat = GL_DEPTH_COMPONENT32);
	static void destroyTexture(uint32_t id) { if (id) glDeleteTextures(1, &id); }

	static uint64_t createTextureHandle(uint32_t textureID);
	static void checkTextureHandle(uint64_t handle);
	static void destroyTextureHandle(uint64_t handle) { GLCall(glMakeTextureHandleNonResidentARB(handle)); }

	static uint64_t createImageHandle(uint32_t textureID, GLenum accessFormat, GLenum accessMode);
	static void checkImageHandle(uint64_t handle);
	static void destroyImageHandle(uint64_t handle) { GLCall(glMakeImageHandleNonResidentARB(handle)); }

	static uint32_t loadTextureFromFilepath(string path);
	static uint32_t loadTextureFromFilepath(string path, int* w, int* h);
	static uint32_t loadTextureFromModel(const aiTexture* aiTex);
	static uint32_t loadCubemapFromFilepath(vector<string> faces);

	static void saveTextureToJPG(GLuint textureID, int width, int height, const std::string& outputPath);

	static string textureTypeToString(aiTextureType type);
	static void printAllTextureType(aiMaterial* mat);

	//static void bindTexture(uint32_t textureID, unsigned int shader_slot = 0);
	//static void bindCubemap(uint32_t textureID, unsigned int shader_slot = 0);
	//static void bindImageTexture(uint32_t textureID, unsigned int unit, GLenum access, GLenum accessFormat = GL_RGBA8);
};