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
bool vertical = false;
bool tangnetSpace = true;

string directory = {
	"F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/" };
inline string getPath(const string& relativePath)
{
	return directory + relativePath;
}

int main()
{
	Window window(WIDTH, HEIGHT, "还不如去玩原神", vec3(0, 2, 3));
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model mStone(getPath("res/object/ground/stone/stone.obj"));
	Model mLight(getPath("res/object/cube/cube.obj"));

	float lightY = 2;
	float lightZ = 3;
	vec3 lightPos(0, lightY, lightZ);
	vec3 lightColor(1);

	Shader sPrev(getPath("shader/preview.shader"));
	Shader sPhongTangent(
		getPath("shader/phong/normalMap/normalMap_Tangent.vert"),
		getPath("shader/phong/normalMap/normalMap_Tangent.frag"));
	Shader sPhong(
		getPath("shader/phong/normalMap/normalMap.vert"),
		getPath("shader/phong/normalMap/normalMap.frag"));
	Shader sLight(getPath("shader/color.shader"));

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
		if (vertical)
		{
			modelMatrix = rotate(modelMatrix, radians(90.f), vec3(1, 0, 0));
			lightPos.y = -lightY;
			lightPos.z = lightZ;
		}
		else
		{
			modelMatrix = mat4(1);
			lightPos.y = lightY;
			lightPos.z = -lightZ;
		}
		if (tangnetSpace)
		{
			sPhongTangent.bind();
			sPhongTangent.bindBlockUBO(ubo.getUBO(), 0);
			sPhongTangent.setMat4("model", modelMatrix);
			sPhongTangent.setVec3("viewPos", camera.position);
			sPhongTangent.setVec3("light.pos", lightPos);
			sPhongTangent.setVec3("light.color", lightColor);
			mStone.draw(sPhongTangent);
		}
		else
		{
			sPhong.bind();
			sPhong.bindBlockUBO(ubo.getUBO(), 0);
			sPhong.setMat4("model", modelMatrix);
			sPhong.setVec3("viewPos", camera.position);
			sPhong.setVec3("light.pos", lightPos);
			sPhong.setVec3("light.color", lightColor);
			mStone.draw(sPhong);
		}

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
		ImGui::Checkbox("Tangnet Space", &tangnetSpace);
		ImGui::Checkbox("Set Vertical", &vertical);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
