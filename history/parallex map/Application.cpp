#include <glm.hpp>

#include <iostream>
#include <string>
#include <map>

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
bool divideByZ = false;
float heightScale = 0.1;
int parallaxMode = 0;
const char* modes[] = {
	"No Parallax Mapping", "Parallax Mapping", "Steep Parallax Mapping", "Parallax Occlusion Mapping" };
int maxLayers = 32;

string directory = {
	"F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/" };
inline string getPath(const string& relativePath)
{
	return directory + relativePath;
}

int main()
{
	Window window(WIDTH, HEIGHT, "还不如去玩原神", vec3(0, 0, 3));
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model mBrick(getPath("res/object/ground/brick/brick.obj"));
	Model mLight(getPath("res/object/cube/cube.fbx"));

	vec3 lightPos(0, 4, 3);
	vec3 lightColor(1);

	Shader sPrev(getPath("shader/preview.shader"));
	Shader sLight(getPath("shader/color.shader"));
	Shader sPhong(
		getPath("shader/phong/parallaxMap/parallaxMap.vert"),
		getPath("shader/phong/parallaxMap/parallaxMap.frag"));

	UniformBuffer ubo(2 * sizeof(mat4));

	Renderer renderer;
	renderer.setClearColor(0.1, 0.1, 0.1, 1);

	while (!window.shouldClose())
	{
		renderer.clearAllBit();

		window.updateTime();
		window.processInput();
		window.freezeView = freezeView;
		Camera camera = window.getCamera();

		mat4 modelMatrix(1);
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = camera.getProjectionMatrix();
		ubo.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
		ubo.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));

		// stone
		modelMatrix = mat4(1);
		modelMatrix = rotate(modelMatrix, radians(90.f), vec3(1, 0, 0));
		sPhong.bind();
		sPhong.bindBlockUBO(ubo.getUBO(), 0);
		sPhong.setMat4("model", modelMatrix);
		sPhong.setVec3("viewPos", camera.position);
		sPhong.setVec3("lightPos", lightPos);
		sPhong.setVec3("lightColor", lightColor);
		sPhong.setFloat1("heightScale", heightScale);
		sPhong.setInt1("divideByZ", divideByZ);
		sPhong.setInt1("parallaxMode", parallaxMode);
		sPhong.setFloat1("maxLayers", maxLayers);
		mBrick.draw(sPhong);

		// light
		modelMatrix = mat4(1);
		modelMatrix = translate(modelMatrix, lightPos);
		modelMatrix = scale(modelMatrix, 0.1f * vec3(1));
		sLight.bind();
		sLight.bindBlockUBO(ubo.getUBO(), 0);
		sLight.setMat4("model", modelMatrix);
		sLight.setVec3("iColor", lightColor);
		mLight.draw(sLight);

		#pragma region ImGui
		imguiManager.beginFrame("Parameter");

		ImGui::Checkbox("Freeze View", &freezeView);

		//ImGui::Combo("Parallax Mode", &parallaxMode, modes, IM_ARRAYSIZE(modes));
		for (int i = 0; i < IM_ARRAYSIZE(modes); i++)
			if (ImGui::RadioButton(modes[i], parallaxMode == i))
				parallaxMode = i;

		if (parallaxMode != 0)
		{
			ImGui::Checkbox("Divide By Z", &divideByZ);
			ImGui::SliderFloat("Height Scale", &heightScale, 0, 1);
			ImGui::SliderInt("Max Layers", &maxLayers, 10, 100);
		}

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
