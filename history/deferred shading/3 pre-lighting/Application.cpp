#include <glm.hpp>

#include <iostream>
#include <map>
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
#define PI 3.14159265359f

bool freezeView = false;
bool usePreLighting = true;
float R_max = 4.0;

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

int main()
{
	Window window(WIDTH, HEIGHT);
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model mNanosuit(getPath("res/object/nanosuit/nanosuit.obj"));
	Model mLight(getPath("res/object/sphere/sphere.fbx"));

	const int lightCount = 1000;
	vector<vec3> lightPos(lightCount);
	vector<vec3> lightColor(lightCount);
	vector<pair<vec3, vec3>> lightData;
	lightData.reserve(lightCount);
	for (int i = 0; i < lightCount; i++)
	{
		lightPos[i] = randomPoint(10);
		lightColor[i] = randomColor(0.2);
		lightData.emplace_back(lightPos[i], lightColor[i]);
	}
	VertexBufferLayout layout;
	layout.push<float>(3);
	layout.push<float>(3);
	mLight.addInstanceData(lightData.data(), lightData.size() * sizeof(lightData[0]), layout);

	ScreenQuad screenQuad;

	Shader sPrev(getPath("shader/preview.shader"));
	Shader sScreen(getPath("shader/screen.shader"));
	Shader sDrawLight(getPath("shader/deferred/instancedLight.shader"));
	Shader sRenderGbuffer(
		getPath("shader/deferred/renderGbuffer.vert"),
		getPath("shader/deferred/renderGbuffer.frag"));
	Shader sLightVolume(
		getPath("shader/deferred/lightVolume/lightVolume.vert"),
		getPath("shader/deferred/lightVolume/lightVolume.frag"));
	Shader sLightingPass(
		getPath("shader/deferred/preLighting/lightingPass.vert"),
		getPath("shader/deferred/preLighting/lightingPass.frag"));
	Shader sShadingPass(
		getPath("shader/deferred/preLighting/shadingPass.vert"),
		getPath("shader/deferred/preLighting/shadingPass.frag"));

	UniformBuffer ubo(2 * sizeof(mat4));

	FrameBuffer G_Buffer(WIDTH, HEIGHT, FrameBufferType::G_BUFFER);
	FrameBuffer L_Buffer(WIDTH, HEIGHT, FrameBufferType::COLOR_FLOAT, 2);

	Renderer renderer;
	renderer.setClearColor(vec3(0), 1.0);

	while (!window.shouldClose())
	{
		window.updateTime();
		window.processInput();
		window.freezeView = freezeView;
		Camera camera = window.getCamera();

		mat4 modelMatrix(1);
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = camera.getProjectionMatrix();
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
		}

		if (usePreLighting)
		{
			// lighting pass (with light volume)
			L_Buffer.bind();
			renderer.clearAllBit();
			renderer.d_DepthWrite();
			renderer.e_BlendAdd();

			G_Buffer.bindColorTex(0, 0); // position
			G_Buffer.bindColorTex(1, 1); // normal
			sLightingPass.bind();
			sLightingPass.bindUBO(ubo.getUBO(), 0);
			sLightingPass.setInt1("gPosition", 0);
			sLightingPass.setInt1("gNormal", 1);
			sLightingPass.setVec3("viewPos", camera.position);
			sLightingPass.setFloat1("R_max", R_max);

			renderer.e_CullFaceFront();
			mLight.draw(sLightingPass, lightCount);
			renderer.e_CullFaceBack();

			renderer.d_Blend();

			// shading pass
			FrameBuffer::bindDefault();
			renderer.clearAllBit();

			L_Buffer.bindColorTex(0, 0); // diffuse light
			L_Buffer.bindColorTex(1, 1); // specular light
			G_Buffer.bindColorTex(2, 2); // albedo - spec
			sShadingPass.bind();
			screenQuad.draw(sShadingPass);

			renderer.e_DepthWrite();
		}
		else
		{
			// light volume
			FrameBuffer::bindDefault();
			renderer.clearAllBit();
			renderer.d_DepthWrite();
			renderer.e_BlendAdd();

			G_Buffer.bindColorTex(0, 0); // position
			G_Buffer.bindColorTex(1, 1); // normal
			G_Buffer.bindColorTex(2, 2); // albedo - spec
			sLightVolume.bind();
			sLightVolume.bindUBO(ubo.getUBO(), 0);
			sLightVolume.setInt1("gPosition", 0);
			sLightVolume.setInt1("gNormal", 1);
			sLightVolume.setInt1("gAlbedoSpec", 2);
			sLightVolume.setVec3("viewPos", camera.position);
			sLightVolume.setFloat1("R_max", R_max);

			renderer.e_CullFaceFront();
			mLight.draw(sLightVolume, lightCount);
			renderer.e_CullFaceBack();

			renderer.d_Blend();
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

		#pragma region ImGui
		imguiManager.beginFrame("Parameter");

		ImGui::Checkbox("Freeze View", &freezeView);
		ImGui::Checkbox("Use Pre Lighting", &usePreLighting);
		ImGui::SliderFloat("Max Atten Radius", &R_max, 0, 10);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
