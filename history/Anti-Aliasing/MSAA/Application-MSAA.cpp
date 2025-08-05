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

using namespace glm;
using namespace std;

#define WIDTH 800
#define HEIGHT 800


int main()
{
	Window window(WIDTH, HEIGHT, "还不如去玩原神");
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model model_cube("res/object/cube/cube.fbx");

	Shader shader_prev("shader/preview.shader");
	Shader shader_cube("shader/light.shader");

	UniformBuffer ubo(2 * sizeof(mat4));

	ScreenQuad screenQuad;
	FrameBuffer fbo_ms(WIDTH, HEIGHT, false, true);
	FrameBuffer fbo(WIDTH, HEIGHT, false, false);
	Shader shader_screen("shader/postFX.vert", "shader/postFX_Gray.frag");
	shader_screen.bind();
	shader_screen.setInt1("screenTexture", 0);

	Renderer renderer;
	renderer.setClearColor(0.05, 0.05, 0.05, 1);

	bool useMSAA = false;
	bool useFBO = false;

	while (!window.shouldClose())
	{
		if (useFBO)
		{
			if (useMSAA) fbo_ms.bind();
			else fbo.bind();
		}
		renderer.clearAllBit();

		window.updateTime();
		window.processInput();
		Camera camera = window.getCamera();

		mat4 modelMatrix = mat4(1);
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = perspective(radians(camera.fov), window.getAspectRatio(), 0.1f, 300.0f);

		ubo.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
		ubo.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));

		{
			ubo.bindPoint(shader_cube, "Matrices", 0);
			shader_cube.bind();
			shader_cube.setMat4("model", modelMatrix);
			model_cube.draw(shader_cube);
		}

		if (useFBO)
		{
			if (useMSAA) 
				fbo.blitFrom(fbo_ms.getFrameBufferID());
			fbo.unbind();
			fbo.bindColorTex();
			renderer.clearColorBit();
			renderer.disableDepthTest();
			screenQuad.draw(shader_screen);
			renderer.enableDepthTest();
		}

		#pragma region ImGui
		imguiManager.beginFrame("Paramater");

		ImGui::Checkbox("use fbo", &useFBO);
		ImGui::Checkbox("use MSAA", &useMSAA);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
