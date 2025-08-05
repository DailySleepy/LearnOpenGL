#include <glm.hpp>
#include <iostream>
#include <string>

#include "Camera.h"
#include "FrameBuffer.h"
#include "ImGuiManager.h"
#include "Model.h"
#include "ScreenQuad.h"
#include "Shader.h"
#include "StorageBuffer.h"
#include "UniformBuffer.h"
#include "Window.h"

using namespace glm;
using namespace std;

#define WIDTH 1200
#define HEIGHT 1200
#define TILE_SIZE 32
#define PI 3.14159265359f

const int lightCount = 1024;
const int maxLightsPerCluster = 512;
const int tileCountX = ceil(float(WIDTH) / TILE_SIZE); // 38
const int tileCountY = ceil(float(HEIGHT) / TILE_SIZE); // 38
const int depthSliceCount = 16;
const int clusterCount = tileCountX * tileCountY * depthSliceCount;

float R_max = 4.0;
bool freezeView = false;
bool enablePlaneIntersection = true;
bool enableAABBIntersection = true;
bool enableLogDepthSlice = false;

string directory = { "F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/" };
inline string getPath(const string& relativePath)
{
	return directory + relativePath;
}

vec3 randomPoint(float radius)
{
	float radius2 = radius * 2;
	float x = static_cast<float>(rand()) / RAND_MAX * radius2 - radius;
	float y = static_cast<float>(rand()) / RAND_MAX * radius2 - radius;
	float z = static_cast<float>(rand()) / RAND_MAX * radius2 - radius;
	return vec3(x, y, z);
}
vec3 randomColor(float minVal)
{
	float r, g, b;
	do
	{
		r = static_cast<float>(rand()) / RAND_MAX;
		g = static_cast<float>(rand()) / RAND_MAX * 0.85;
		b = static_cast<float>(rand()) / RAND_MAX;
	} while (r < minVal && g < minVal && b < minVal);
	return vec3(r, g, b);
}
pair<vector<vec4>, vector<vec4>> getLightData(int lightCount)
{
	vector<vec4> lightPos(lightCount), lightColor(lightCount);

	for (int i = 0; i < lightCount; i++)
	{
		lightPos[i] = vec4(randomPoint(10), 1);
		lightColor[i] = vec4(randomColor(0.2), 1);
	}
	return make_pair(lightPos, lightColor);
}

int main()
{
	Window window(WIDTH, HEIGHT);
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model mNanosuit(getPath("res/object/nanosuit/nanosuit.obj"));
	Model mLight(getPath("res/object/sphere/sphere.fbx"));
	Model mRoom(getPath("res/object/room/room.obj"));
	Model mGlass(getPath("res/object/glass/glass.obj"));

	ScreenQuad screenQuad;

	#pragma region Shaders
	Shader sPrev(getPath("shader/preview.shader"));
	Shader sScreen(getPath("shader/screen.shader"));
	Shader sDrawLight(getPath("shader/deferred/instancedLight.shader"));
	Shader sPhong(getPath("shader/phong/phong_multiPointLight.shader"));
	Shader sRenderGbuffer(
		getPath("shader/deferred/renderGbuffer.vert"),
		getPath("shader/deferred/renderGbuffer.frag"));
	Shader sTransformLight(
		getPath("shader/deferred/transformLight.comp"), ShaderType::Compute);
	Shader sClusterLight(
		getPath("shader/deferred/clustered/clusterLight.comp"), ShaderType::Compute);
	Shader sClusteredDeferred(
		getPath("shader/deferred/clustered/clusteredDeferred.vert"),
		getPath("shader/deferred/clustered/clusteredDeferred.frag"));
	#pragma endregion

	int lightDataSize = lightCount * sizeof(vec4);
	auto [lightPos, lightColor] = getLightData(lightCount);
	#pragma region addInstanceData
	VertexBufferLayout layout;
	layout.push<float>(4);
	mLight.addInstanceData(lightPos.data(), lightDataSize, layout);
	mLight.addInstanceData(lightColor.data(), lightDataSize, layout);
	#pragma endregion
	StorageBuffer ssbo_lightPos(lightDataSize, lightPos.data(), GL_STATIC_DRAW);
	StorageBuffer ssbo_lightColor(lightDataSize, lightColor.data(), GL_STATIC_DRAW);
	StorageBuffer ssbo_lightPosView(lightDataSize);
	StorageBuffer ssbo_lightIndices(clusterCount * maxLightsPerCluster * sizeof(uint32_t));
	//StorageBuffer ssbo_debug(clusterCount * sizeof(vec2));

	UniformBuffer ubo(2 * sizeof(mat4));

	FrameBuffer G_Buffer(WIDTH, HEIGHT, FrameBufferType::G_BUFFER);

	Renderer renderer;
	renderer.setClearColor(vec3(0), 0);

	while (!window.shouldClose())
	{
		window.updateTime();
		window.processInput();
		window.freezeView = freezeView;
		Camera camera = window.getCamera();

		mat4 modelMatrix(1);
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = camera.getProjectionMatrix();
		mat4 invProj = inverse(projectionMatrix);
		ubo.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
		ubo.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));

		// render G buffer
		{
			G_Buffer.bind();
			renderer.clearAllBit();

			modelMatrix = mat4(1);
			sRenderGbuffer.bind();
			sRenderGbuffer.bindUBO(ubo.getUBO(), 0);
			for (int i = -1; i <= 1; ++i)
			{
				for (int j = -1; j <= 1; ++j)
				{
					modelMatrix = mat4(1.0f);
					modelMatrix = translate(modelMatrix, vec3(i * 3.0f, -1.0f, j * 3.0f));
					modelMatrix = scale(modelMatrix, 0.2f * vec3(1));
					sRenderGbuffer.setMat4("model", modelMatrix);
					mNanosuit.draw(sRenderGbuffer);
				}
			}
			modelMatrix = mat4(1);
			sRenderGbuffer.setMat4("model", modelMatrix);
			mRoom.draw(sRenderGbuffer);
		}

		// compute lightPosView
		{
			sTransformLight.bind();
			sTransformLight.bindUBO(ubo.getUBO(), 0);
			sTransformLight.bindSSBO(ssbo_lightPos.getSSBO(), 0);
			sTransformLight.bindSSBO(ssbo_lightPosView.getSSBO(), 1);
			sTransformLight.setInt1("lightCount", lightCount);
			glDispatchCompute(ceil(float(lightCount) / 64), 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}

		// compute cluster light list
		{
			sClusterLight.bind();
			sClusterLight.bindSSBO(ssbo_lightPosView.getSSBO(), 0);
			sClusterLight.bindSSBO(ssbo_lightIndices.getSSBO(), 1);
			sClusterLight.setMat4("invProj", invProj);
			sClusterLight.setFloat2("screenSize", WIDTH, HEIGHT);
			sClusterLight.setFloat1("R_max", R_max);
			sClusterLight.setInt1("lightCount", lightCount);
			sClusterLight.setInt1("tileSize", TILE_SIZE);
			sClusterLight.setInt1("tileCountX", tileCountX);
			sClusterLight.setInt1("tileCountY", tileCountY);
			sClusterLight.setInt1("depthSliceCount", depthSliceCount);
			sClusterLight.setInt1("clusterCount", clusterCount);
			sClusterLight.setInt1("maxLightsPerCluster", maxLightsPerCluster);
			sClusterLight.setFloat1("z_near", camera.zNear);
			sClusterLight.setFloat1("z_far", camera.zFar);
			sClusterLight.setInt1("enablePlaneIntersection", enablePlaneIntersection);
			sClusterLight.setInt1("enableAABBIntersection", enableAABBIntersection);
			sClusterLight.setInt1("enableLogDepthSlice", enableLogDepthSlice);
			glDispatchCompute(ceil(float(clusterCount) / 64), 1, 1);
			//glDispatchCompute(ceil(float(tileCountX) / 8), ceil(float(tileCountY) / 8), ceil(float(depthSliceCount) / 4));
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}

		// render clusters
		{
			FrameBuffer::bindDefault();
			renderer.clearAllBit();
			renderer.d_DepthWrite();

			sClusteredDeferred.bind();
			sClusteredDeferred.bindUBO(ubo.getUBO(), 0);
			sClusteredDeferred.bindSSBO(ssbo_lightPos.getSSBO(), 0);
			sClusteredDeferred.bindSSBO(ssbo_lightColor.getSSBO(), 1);
			sClusteredDeferred.bindSSBO(ssbo_lightIndices.getSSBO(), 2);
			sClusteredDeferred.setHandle("gPosition", G_Buffer.getColorTextureHandle(0));
			sClusteredDeferred.setHandle("gNormal", G_Buffer.getColorTextureHandle(1));
			sClusteredDeferred.setHandle("gAlbedoSpec", G_Buffer.getColorTextureHandle(2));
			sClusteredDeferred.setHandle("gDepth", G_Buffer.getDepthTextureHandle());
			sClusteredDeferred.setInt1("tileSize", TILE_SIZE);
			sClusteredDeferred.setInt1("tileCountX", tileCountX);
			sClusteredDeferred.setInt1("tileCountY", tileCountY);
			sClusteredDeferred.setInt1("depthSliceCount", depthSliceCount);
			sClusteredDeferred.setInt1("maxLightsPerCluster", maxLightsPerCluster);
			sClusteredDeferred.setVec3("viewPos", camera.position);
			sClusteredDeferred.setFloat1("z_near", camera.zNear);
			sClusteredDeferred.setFloat1("z_far", camera.zFar);
			sClusteredDeferred.setFloat1("R_max", R_max);
			sClusteredDeferred.setInt1("enableLogDepthSlice", enableLogDepthSlice);
			screenQuad.draw(sClusteredDeferred);

			renderer.e_DepthWrite();
		}

		// draw lights
		{
			FrameBuffer::blit(GL_DEPTH_BUFFER_BIT, G_Buffer.getFBO(), WIDTH, HEIGHT, 0, WIDTH, HEIGHT);

			sDrawLight.bind();
			sDrawLight.bindUBO(ubo.getUBO(), 0);
			sDrawLight.setFloat1("radius", 0.05f);
			mLight.draw(sDrawLight, lightCount);
		}

		// draw transparent
		{
			renderer.e_BlendAlpha();
			renderer.d_CullFace();

			modelMatrix = mat4(1);
			modelMatrix = translate(modelMatrix, vec3(1, 1, 5));
			modelMatrix = scale(modelMatrix, 0.5f * vec3(1));

			sPhong.bind();
			sPhong.bindUBO(ubo.getUBO(), 0);
			sPhong.bindSSBO(ssbo_lightPos.getSSBO(), 0);
			sPhong.bindSSBO(ssbo_lightColor.getSSBO(), 1);
			sPhong.setMat4("model", modelMatrix);
			sPhong.setVec3("viewPos", camera.position);
			sPhong.setInt1("lightCount", lightCount);
			sPhong.setFloat1("R_max", R_max);
			mGlass.draw(sPhong);

			renderer.d_Blend();
			renderer.e_CullFace();
		}

	renderingOver:
		#pragma region ImGui
		imguiManager.beginFrame("Parameter");

		ImGui::Checkbox("Freeze View", &freezeView);
		ImGui::SliderFloat("Max Light Radius", &R_max, 0, 10);
		ImGui::Checkbox("Enable AABB Intersection", &enableAABBIntersection);
		ImGui::Checkbox("Enable Plane Intersection", &enablePlaneIntersection);
		ImGui::Checkbox("Enable Logarithmic Depth Slicing", &enableLogDepthSlice);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
