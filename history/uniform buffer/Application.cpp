#include <glm.hpp>

#include <iostream>
#include <string>
#include <map>

#include "Window.h"
#include "ImGuiManager.h"
#include "Camera.h"
#include "Model.h"
#include "Shader.h"
#include "FBOManager.h"
#include "UniformBuffer.h"

using namespace glm;
using namespace std;

#define WIDTH 800
#define HEIGHT 800


int main()
{
	Window window(WIDTH, HEIGHT, "学OpenGL的这辈子有了");
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model model_cube("res/object/cube/cube.fbx");
	Shader shader_light0("shader/light.shader");
	Shader shader_light1("shader/light1.shader");
	Shader shader_light2("shader/light2.shader");
	Shader shader_light3("shader/light3.shader");
	vec3 color[4] = {
		vec3(1.0, 0.0, 0.0), // 红色
		vec3(0.0, 1.0, 0.0), // 绿色
		vec3(0.0, 0.0, 1.0), // 蓝色
		vec3(1.0, 1.0, 0.0)  // 黄色
	};
	glm::vec3 position[4] = {
		glm::vec3(-1.0f,  1.0f,  0.0f), // 左上角
		glm::vec3(1.0f,  1.0f,  0.0f), // 右上角
		glm::vec3(-1.0f, -1.0f,  0.0f), // 左下角
		glm::vec3(1.0f, -1.0f,  0.0f)  // 右下角
	};


	UniformBuffer uboMatrices(2 * sizeof(mat4));
	uboMatrices.bindPoint(shader_light0, "Matrices", 0);
	uboMatrices.bindPoint(shader_light1, "Matrices", 0);
	uboMatrices.bindPoint(shader_light2, "Matrices", 0);
	uboMatrices.bindPoint(shader_light3, "Matrices", 0);

	Renderer renderer;
	renderer.setClearColor(0.05, 0.05, 0.05, 1);

	while (!window.shouldClose())
	{
		renderer.clearAllBit();

		window.updateTime();
		window.processInput();
		Camera camera = window.getCamera();

		mat4 modelMatrix = mat4(1);
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = perspective(radians(camera.fov), window.getAspectRatio(), 0.1f, 100.0f);

		uboMatrices.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
		uboMatrices.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));

		{
			modelMatrix = translate(modelMatrix, position[0]);
			modelMatrix = scale(modelMatrix, 0.3f * vec3(1));

			shader_light0.bind();
			shader_light0.setMatrix4("model", modelMatrix);
			shader_light0.setVec3("iColor", color[0]);
			model_cube.draw(shader_light0);
		}
		{
			modelMatrix = translate(mat4(1.0f), position[1]);
			modelMatrix = scale(modelMatrix, 0.3f * vec3(1));

			shader_light1.bind();
			shader_light1.setMatrix4("model", modelMatrix);
			shader_light1.setVec3("iColor", color[1]);
			model_cube.draw(shader_light1);
		}
		{
			modelMatrix = translate(mat4(1.0f), position[2]);
			modelMatrix = scale(modelMatrix, 0.3f * vec3(1));

			shader_light2.bind();
			shader_light2.setMatrix4("model", modelMatrix);
			shader_light2.setVec3("iColor", color[2]);
			model_cube.draw(shader_light2);
		}
		{
			modelMatrix = translate(mat4(1.0f), position[3]);
			modelMatrix = scale(modelMatrix, 0.3f * vec3(1));

			shader_light3.bind();
			shader_light3.setMatrix4("model", modelMatrix);
			shader_light3.setVec3("iColor", color[3]);
			model_cube.draw(shader_light3);
		}

		#pragma region ImGui
		imguiManager.beginFrame("Paramater");

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}