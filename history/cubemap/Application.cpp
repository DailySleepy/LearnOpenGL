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
	Window window(WIDTH, HEIGHT, "学OpenGL的这辈子有了");
	ImGuiManager imguiManager(window.getGLFWWindow());

	//Model model_kfk("res/object/kfk/kfk.obj");
	//Shader shader("shader/preview.shader");

	Model model_nanosuit("res/object/nanosuit/nanosuit.obj");
	Shader shader_reflection("shader/reflection_nanosuit.vert", "shader/reflection_nanosuit.frag");

	Model model_cube("res/object/cube/cube.fbx");
	vector<string> faces{
		"res/texture/skybox/right.jpg",
		"res/texture/skybox/left.jpg",
		"res/texture/skybox/top.jpg",
		"res/texture/skybox/bottom.jpg",
		"res/texture/skybox/front.jpg",
		"res/texture/skybox/back.jpg"
	};
	unsigned int cubemapTexture = TEX::loadCubemap(faces);
	TEX::bindCubemap(cubemapTexture, 0);
	Shader shader_skybox("shader/skybox.shader");
	shader_skybox.bind();
	shader_skybox.setInt1("skybox", 0);

	Renderer renderer;
	renderer.setClearColor(0.05, 0.05, 0.05, 1);

	float IOR = 1.5;

	while (!window.shouldClose())
	{
		renderer.clearAllBit();

		window.updateTime();
		window.processInput();
		Camera camera = window.getCamera();

		mat4 modelMatrix = mat4(1);
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = perspective(radians(camera.fov), window.getAspectRatio(), 0.1f, 100.0f);

		{
			// kfk
			//modelMatrix = mat4(1);
			//modelMatrix = translate(modelMatrix, vec3(0, -1, 0));
			//modelMatrix = scale(modelMatrix, 1.5f * vec3(1, 1, 1));

			//shader.bind();
			//shader.setMatrix4("model", modelMatrix);
			//shader.setMatrix4("view", viewMatrix);
			//shader.setMatrix4("projection", projectionMatrix);

			//model_kfk.draw(shader);
		}
		{
			// reflection
			modelMatrix = mat4(1);
			modelMatrix = translate(modelMatrix, vec3(0, -3, 0));
			modelMatrix = scale(modelMatrix, 0.3f * vec3(1, 1, 1));
			shader_reflection.bind();
			shader_reflection.setMatrix4("model", modelMatrix);
			shader_reflection.setMatrix4("view", viewMatrix);
			shader_reflection.setMatrix4("projection", projectionMatrix);
			shader_reflection.setInt1("skybox", 1); // 不能使用0, 已经被diffuse使用
			shader_reflection.setVec3("cameraPos", camera.position);
			model_nanosuit.draw(shader_reflection);
		}
		{
			// skybox
			shader_skybox.bind();
			shader_skybox.setMatrix4("view", mat4(mat3(viewMatrix)));
			shader_skybox.setMatrix4("projection", projectionMatrix);

			renderer.disableCullFace();
			renderer.enableDepthTestLEqual();
			model_cube.draw(shader_skybox);
			renderer.enableCullFace();
			renderer.enableDepthTestLess();
		}

		#pragma region ImGui
		imguiManager.beginFrame("Paramater");

		ImGui::SliderFloat("IOR", &IOR, 1.0, 10.0);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}