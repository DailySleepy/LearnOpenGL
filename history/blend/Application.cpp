#include <glm.hpp>

#include <iostream>
#include <string>
#include <map>

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

	Model model_kfk("res/object/mihoyo/kfk/kfk.obj");
	Model model_ground("res/object/grass/grassground.obj");
	Shader shader("shader/preview.shader");

	Model model_sphere("res/object/sphere/sphere.fbx");
	Shader lightShader("shader/light.shader");

	float vertices[] = {
	-0.5f, 0, 0,  0.0f, 0.0f,
	 0.5f, 0, 0,  1.0f, 0.0f,
	 0.5f, 1, 0,  1.0f, 1.0f,
	-0.5f, 1, 0,  0.0f, 1.0f,
	};
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};
	vector<vec3> position = {
		vec3(-1.5f, 0.0f, -0.48f),
		vec3(1.5f, 0.0f, 0.51f),
		vec3(0.0f, 0.0f, 0.7f),
		vec3(-0.3f, 0.0f, -2.3f),
		vec3(0.5f, 0.0f, -0.6f),
	};
	VertexArray vao;
	VertexBuffer vbo(vertices, sizeof(vertices));
	VertexBufferLayout layout;
	layout.push<float>(3);
	layout.push<float>(2);
	vao.AddBuffer(layout);
	IndexBuffer ibo(indices, sizeof(indices) / sizeof(indices[0]));
	Shader transparentShader("shader/transparent.shader");
	unsigned int glassTexture = TEX::loadTextureFromFilepath("res/texture/transparent_window.png", GL_CLAMP_TO_EDGE);

	float lightColor[3] = { 1.0f, 1.0f, 1.0f };
	vec3 lightPos = vec3(2, 1, 2);

	while (!window.shouldClose())
	{
		renderer.clear();

		window.updateTime();
		window.processInput();
		Camera camera = window.getCamera();

		mat4 modelMatrix;
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = perspective(radians(camera.fov), window.getAspectRatio(), 0.1f, 100.0f);
		{
			modelMatrix = mat4(1);
			modelMatrix = translate(modelMatrix, lightPos);
			modelMatrix = scale(modelMatrix, 0.2f * vec3(1, 1, 1));

			lightShader.bind();
			lightShader.setUniform4M("model", modelMatrix);
			lightShader.setUniform4M("view", viewMatrix);
			lightShader.setUniform4M("projection", projectionMatrix);
			lightShader.setUniform3f("iColor", lightColor[0], lightColor[1], lightColor[2]);

			model_sphere.draw(lightShader);
		}
		{
			modelMatrix = mat4(1);
			modelMatrix = translate(modelMatrix, vec3(0, -1, 0));
			modelMatrix = scale(modelMatrix, 1.5f * vec3(1, 1, 1));

			shader.bind();
			shader.setUniform4M("model", modelMatrix);
			shader.setUniform4M("view", viewMatrix);
			shader.setUniform4M("projection", projectionMatrix);

			model_kfk.draw(shader);
		}
		{
			modelMatrix = mat4(1);
			modelMatrix = rotate(modelMatrix, radians(180.0f), vec3(0, 1, 0));

			shader.bind();
			shader.setUniform4M("model", modelMatrix);
			shader.setUniform4M("view", viewMatrix);
			shader.setUniform4M("projection", projectionMatrix);

			model_ground.draw(shader);
		}
		{
			map<float, vec3> sorted_pos;
			for (int i = 0; i < position.size(); i++)
			{
				float distance = length(camera.position - position[i]);
				sorted_pos[distance] = position[i];
			}

			for (auto it = sorted_pos.rbegin(); it != sorted_pos.rend(); it++)
			{
				modelMatrix = mat4(1);
				vec3 pos = it->second;
				modelMatrix = translate(modelMatrix, pos);

				transparentShader.bind();
				transparentShader.setUniform4M("model", modelMatrix);
				transparentShader.setUniform4M("view", viewMatrix);
				transparentShader.setUniform4M("projection", projectionMatrix);
				unsigned int slot = 0;
				TEX::bindTexture(slot, glassTexture);
				transparentShader.setUniform1i("texture1", slot);

				renderer.draw(vao, vbo, ibo, transparentShader);
			}
		}

		#pragma region ImGui
		imguiManager.beginFrame("Shader Para");

		ImGui::ColorEdit3("light color", lightColor);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}