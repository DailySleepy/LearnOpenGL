#include <glm.hpp>

#include <iostream>
#include <string>

#include "Window.h"
#include "ImGuiManager.h"
#include "Camera.h"
#include "Model.h"
#include "Shader.h"

using namespace glm;
using namespace std;

#define WIDTH 800
#define HEIGHT 800


int main()
{
	Window window(WIDTH, HEIGHT, "Hello World");
	ImGuiManager imguiManager(window.getGLFWWindow());

	Renderer renderer;
	renderer.setBackground(0.05, 0.05, 0.05, 1);

	Model model("res/object/mihoyo/kfk/kfk.obj");
	Shader shader("shader/preview.shader");
	Shader shader_singleColor("shader/singleColor.shader");
	//Shader shader("shader/phong.vert", "shader/phong.frag");

	Model model_sphere("res/object/sphere/sphere.fbx");
	Shader lightShader("shader/light.shader");

	float lightColor[3] = { 1.0f, 1.0f, 1.0f };
	vec3 lightPos = vec3(2, 0, 2);

	bool showOutline = true;

	while (!window.shouldClose())
	{
		renderer.clear();

		window.updateTime();
		window.processInput();
		Camera camera = window.getCamera();

		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = perspective(radians(camera.fov), window.getAspectRatio(), 0.1f, 100.0f);
		{
			mat4 modelMatrix = mat4(1);
			modelMatrix = translate(modelMatrix, lightPos);
			modelMatrix = scale(modelMatrix, 0.2f * vec3(1, 1, 1));

			lightShader.bind();
			lightShader.setUniform4M("model", modelMatrix);
			lightShader.setUniform4M("view", viewMatrix);
			lightShader.setUniform4M("projection", projectionMatrix);
			lightShader.setUniform3f("iColor", lightColor[0], lightColor[1], lightColor[2]);

			glStencilMask(0x00);
			model_sphere.draw(lightShader);
		}
		{
			mat4 modelMatrix = mat4(1);
			modelMatrix = translate(modelMatrix, vec3(0, -1, 0));
			modelMatrix = scale(modelMatrix, 1.5f * vec3(1, 1, 1));

			shader.bind();
			shader.setUniform4M("model", modelMatrix);
			shader.setUniform4M("view", viewMatrix);
			shader.setUniform4M("projection", projectionMatrix);

			glStencilFunc(GL_ALWAYS, 1, 0xff);
			glStencilMask(0xff);
			model.draw(shader);

			if (showOutline)
			{
				mat4 modelMatrix = mat4(1);
				modelMatrix = translate(modelMatrix, vec3(0, -1.03, 0));
				modelMatrix = scale(modelMatrix, 1.525f * vec3(1, 1, 1));

				shader_singleColor.bind();
				shader_singleColor.setUniform4M("model", modelMatrix);
				shader_singleColor.setUniform4M("view", viewMatrix);
				shader_singleColor.setUniform4M("projection", projectionMatrix);

				glStencilFunc(GL_NOTEQUAL, 1, 0xff);
				glStencilMask(0x00);
				glDisable(GL_DEPTH_TEST);
				model.draw(shader_singleColor);
				glStencilMask(0xFF);
				glEnable(GL_DEPTH_TEST);
			}
		}

		#pragma region ImGui
		imguiManager.beginFrame("Shader Para");

		ImGui::ColorEdit3("light color", lightColor);
		ImGui::Checkbox("Show Outline", &showOutline);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}