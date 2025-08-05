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

using namespace glm;
using namespace std;

#define WIDTH 800
#define HEIGHT 800


int main()
{
	Window window(WIDTH, HEIGHT, "不如Vulkan一根");
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model model_kfk("res/object/kfk/kfk.obj");
	Model model_ground("res/object/grass/grassground.obj");
	Shader shader("shader/preview.shader");

	Model model_sphere("res/object/sphere/sphere.fbx");
	Shader lightShader("shader/light.shader");

	float vertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,  // 左上角
		-1.0f, -1.0f,  0.0f, 0.0f,  // 左下角
		 1.0f, -1.0f,  1.0f, 0.0f,  // 右下角
		 1.0f,  1.0f,  1.0f, 1.0f   // 右上角
	};
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};
	VertexArray vao;
	VertexBuffer vbo(vertices, sizeof(vertices));
	VertexBufferLayout layout;
	layout.push<float>(2);
	layout.push<float>(2);
	vao.AddBuffer(layout);
	IndexBuffer ibo(indices, sizeof(indices) / sizeof(indices[0]));
	FrameBuffer fbo(WIDTH, HEIGHT, false, false);
	fbo.unbind();
	Shader screenShader("shader/postFX.vert", "shader/postFX_Sharpening.frag");
	screenShader.bind();
	screenShader.setInt1("screenTexture", 0);

	float lightColor[3] = { 1.0f, 1.0f, 1.0f };
	vec3 lightPos = vec3(2, 1, 2);

	Renderer renderer;
	renderer.setClearColor(0.05, 0.05, 0.05, 1);

	bool postOn = false;

	while (!window.shouldClose())
	{
		if (postOn) fbo.bind();

		renderer.clearAllBit();

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
			lightShader.setMatrix4("model", modelMatrix);
			lightShader.setMatrix4("view", viewMatrix);
			lightShader.setMatrix4("projection", projectionMatrix);
			lightShader.setFloat3("iColor", lightColor[0], lightColor[1], lightColor[2]);
			
			model_sphere.draw(lightShader);
		}
		{
			modelMatrix = mat4(1);
			modelMatrix = translate(modelMatrix, vec3(0, -1, 0));
			modelMatrix = scale(modelMatrix, 1.5f * vec3(1, 1, 1));

			shader.bind();
			shader.setMatrix4("model", modelMatrix);
			shader.setMatrix4("view", viewMatrix);
			shader.setMatrix4("projection", projectionMatrix);

			model_kfk.draw(shader);
		}
		{
			modelMatrix = mat4(1);
			modelMatrix = translate(modelMatrix, vec3(0, -1, 0));
			modelMatrix = rotate(modelMatrix, radians(180.0f), vec3(0, 1, 0));
			
			shader.bind();
			shader.setMatrix4("model", modelMatrix);
			shader.setMatrix4("view", viewMatrix);
			shader.setMatrix4("projection", projectionMatrix);
			
			model_ground.draw(shader);
		}

		if (postOn)
		{
			fbo.unbind();
			fbo.bindColorTex();
			renderer.clearColorBit();
			renderer.disableDepthTest();
			renderer.draw(vao, vbo, ibo, screenShader);
			renderer.enableDepthTest();
		}

		#pragma region ImGui
		imguiManager.beginFrame("Shader Para");

		ImGui::ColorEdit3("light color", lightColor);
		ImGui::Checkbox("use post processing", &postOn);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}