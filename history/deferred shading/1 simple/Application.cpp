#include <glm.hpp>

#include <iostream>
#include <string>
#include <map>
#include <random>

#include "Window.h"
#include "ImGuiManager.h"
#include "Camera.h"
#include "Model.h"
#include "Shader.h"
#include "ScreenQuad.h"
#include "FrameBuffer.h"
#include "UniformBuffer.h"
#include "StorageBuffer.h"

using namespace glm;
using namespace std;

#define WIDTH  1200
#define HEIGHT 1200
#define PI 3.14159265359f

bool freezeView = false;
float R_max = 1;

string directory = {
	"F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/" };
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
		g = static_cast<float>(rand()) / RAND_MAX;
		b = static_cast<float>(rand()) / RAND_MAX;
	} while (r < minVal && g < minVal && b < minVal);
	return vec3(r, g, b);
}

int main()
{
	Window window(WIDTH, HEIGHT, "还不如去玩原神");

	ImGuiManager imguiManager(window.getGLFWWindow());

	Model mNanosuit(getPath("res/object/nanosuit/nanosuit.obj"));
	Model mLight(getPath("res/object/sphere/sphere.fbx"));

	const int lightCount = 1000;
	vector<vec3> lightPos(lightCount);
	vector<vec3> lightColor(lightCount);
	vector<pair<vec3, vec3>> lightData; lightData.reserve(lightCount);
	for (int i = 0; i < lightCount; i++)
	{
		lightPos[i] = randomPoint(10);
		lightColor[i] = randomColor(0.2);
		lightData.emplace_back(lightPos[i], lightColor[i]);
	}
	VertexBufferLayout layout;
	layout.push<float>(3); layout.push<float>(3);
	mLight.addInstanceData(lightData.data(), lightData.size() * sizeof(lightData[0]), layout);

	ScreenQuad screenQuad;

	Shader sPrev(getPath("shader/preview.shader"));
	Shader sScreen(getPath("shader/showOnScreen.shader"));
	Shader sDrawLight(getPath("shader/deferred/instancedLight.shader"));
	Shader sRenderGbuffer(
		getPath("shader/deferred/renderGbuffer.vert"), 
		getPath("shader/deferred/renderGbuffer.frag"));
	Shader sUseGbuffer(
		getPath("shader/deferred/simple/useGbuffer.vert"), 
		getPath("shader/deferred/simple/useGbuffer.frag"));

	UniformBuffer ubo(2 * sizeof(mat4));

	StorageBuffer ssbo_lightPos(lightPos.size() * sizeof(vec3), lightPos.data());
	StorageBuffer ssbo_lightColor(lightColor.size() * sizeof(vec3), lightColor.data());

	sUseGbuffer.bind();
	sUseGbuffer.bindBlockSSBO(ssbo_lightPos.getSSBO(), 0);
	sUseGbuffer.bindBlockSSBO(ssbo_lightColor.getSSBO(), 1);
	sUseGbuffer.unbind();

	FrameBuffer gBuffer(WIDTH, HEIGHT, FrameBufferType::G_BUFFER);

	Renderer renderer;
	renderer.setClearColor(0.1, 0.1, 0.1, 1.0);
	renderer.setClearColor(0.0, 0.0, 0.0, 1.0);

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
		gBuffer.bind();
		renderer.clearAllBit();

		modelMatrix = mat4(1);
		sRenderGbuffer.bind();
		sRenderGbuffer.bindBlockUBO(ubo.getUBO(), 0);
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

		// use G buffer
		FrameBuffer::bindDefault();
		renderer.clearAllBit();
		renderer.disable_DepthWrite();

		sUseGbuffer.bind();
		gBuffer.bindColorTex(0, 0);
		gBuffer.bindColorTex(1, 1);
		gBuffer.bindColorTex(2, 2);
		sUseGbuffer.setInt1("gPosition", 0);
		sUseGbuffer.setInt1("gNormal", 1);
		sUseGbuffer.setInt1("gAlbedoSpec", 2);
		sUseGbuffer.setVec3("viewPos", camera.position);
		sUseGbuffer.setFloat1("R_max", R_max);
		screenQuad.draw(sUseGbuffer);

		// draw lights
		FrameBuffer::blit(GL_DEPTH_BUFFER_BIT, gBuffer.getFBO(), WIDTH, HEIGHT, 0, WIDTH, HEIGHT);
		renderer.enable_DepthWrite();

		sDrawLight.bind();
		sDrawLight.bindBlockUBO(ubo.getUBO(), 0);
		sDrawLight.setFloat1("radius", 0.05f);
		mLight.draw(sDrawLight, lightCount);

		#pragma region ImGui
		imguiManager.beginFrame("Parameter");

		ImGui::Checkbox("Freeze View", &freezeView);
		ImGui::SliderFloat("Max Atten Radius", &R_max, 0, 10);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
