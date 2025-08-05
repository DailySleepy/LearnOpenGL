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
	Window window(WIDTH, HEIGHT, "还不如去玩原神");
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model model_cube("res/object/cube/cube.fbx");
	vector<string> faces = {
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

	Model model_kfk(("res/object/kfk/kfk.obj"));
	Shader shader_prev("shader/preview.shader");
	Shader shader_geom("shader/showNormal.shader");

	UniformBuffer uboMatrices(2 * sizeof(mat4));

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
			uboMatrices.bindPoint(shader_prev, "Matrices", 0);
			shader_prev.bind();
			shader_prev.setMatrix4("model", modelMatrix);
			model_kfk.draw(shader_prev);
		}
		{
			uboMatrices.bindPoint(shader_geom, "Matrices", 0);
			shader_geom.bind();
			shader_geom.setMatrix4("model", modelMatrix);
			model_kfk.draw(shader_geom);
		}
		{
			/*uboMatrices.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(mat4(mat3(viewMatrix))));
			uboMatrices.bindPoint(shader_prev, "Matrices", 0);
			renderer.disableCullFace();
			renderer.enableDepthTestLEqual();
			model_cube.draw(shader_skybox);
			renderer.enableCullFace();
			renderer.enableDepthTestLess();*/
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