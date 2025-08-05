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

#define WIDTH  800
#define HEIGHT 800

int frameCount = 0;
int frameIdx = 0;
bool useTAA = false;
bool useMSAA = false;
bool noAA = true;
bool freezeView = false;
float mixRatio = 0.3;
vec2 jitterOffset = vec2(0.0f);
mat4 prevViewMatrix = mat4(1.0f);
mat4 prevProjectionMatrix = mat4(1.0f);

float halton(int index, int base)
{
	float result = 0.0f;
	float f = 1.0f / base;
	int i = index;
	while (i > 0)
	{
		result += f * (i % base);
		i /= base;
		f /= base;
	}
	return result;
}

void jitterProjection(mat4& proj, int frameIndex, float width, float height)
{
	float jitterX = (halton(frameIndex, 2) - 0.5f) * 2 / width;
	float jitterY = (halton(frameIndex, 3) - 0.5f) * 2 / height;
	proj[2][0] -= jitterX;
	proj[2][1] -= jitterY;
}


int main()
{
	Window window(WIDTH, HEIGHT, "还不如去玩原神");
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model model_cube("res/object/cube/cube.fbx");
	Model model_kfk("res/object/kfk/kfk.obj");

	Shader shader_prev("shader/preview.shader");
	Shader shader_cube("shader/light.shader");

	UniformBuffer ubo(2 * sizeof(mat4));

	ScreenQuad screenQuad;
	FrameBuffer fbo_history[2] = {
		FrameBuffer(WIDTH, HEIGHT, COLOR),
		FrameBuffer(WIDTH, HEIGHT, COLOR)
	};
	FrameBuffer fbo_current(WIDTH, HEIGHT, COLOR);
	FrameBuffer fbo_velocity(WIDTH, HEIGHT, VELOCITY);
	Shader shader_velocity2FBO("shader/velocity2FBO.vert", "shader/velocity2FBO.frag");
	Shader shader_TAA("shader/postFX.vert", "shader/postFX_TAA.frag");
	shader_TAA.bind();
	shader_TAA.setInt1("historyColor", 0);
	shader_TAA.setInt1("currentColor", 1);
	shader_TAA.setInt1("velocity", 2);
	Shader shader_screen("shader/screen.shader");
	shader_screen.bind();
	shader_screen.setInt1("screenTexture", 0);

	Renderer renderer;
	renderer.setClearColor(0.05, 0.05, 0.05, 1);


	while (!window.shouldClose())
	{
		if (!useMSAA) glDisable(GL_MULTISAMPLE);
		else glEnable(GL_MULTISAMPLE);

		renderer.clearAllBit();

		window.updateTime();
		window.processInput();
		Camera camera = window.getCamera();
		window.freezeView = freezeView;

		mat4 modelMatrix = mat4(1);
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = perspective(radians(camera.fov), window.getAspectRatio(), 0.1f, 300.0f);

		if (useTAA)
		{
			fbo_current.bind();
			renderer.clearAllBit();
			jitterProjection(projectionMatrix, frameIdx, WIDTH, HEIGHT);
		}

		ubo.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
		ubo.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));

		{
			shader_cube.bind();
			shader_cube.bindBlock(ubo.getUBO(), "Matrices", 0);
			shader_cube.setMat4("model", modelMatrix);
			shader_cube.setVec3("iColor", vec3(1));
			model_cube.draw(shader_cube);
		}
		{
			shader_prev.bind();
			shader_prev.bindBlock(ubo.getUBO(), "Matrices", 0);
			shader_prev.setMat4("model", modelMatrix);
			model_kfk.draw(shader_prev);
		}

		if (useTAA)
		{
			// velocity
			fbo_velocity.bind();
			renderer.clearAllBit();
			shader_velocity2FBO.bind();
			shader_velocity2FBO.bindBlock(ubo.getUBO(), "Matrices", 0);
			shader_velocity2FBO.setMat4("model", modelMatrix);
			shader_velocity2FBO.setMat4("prevView", prevViewMatrix);
			shader_velocity2FBO.setMat4("prevProjection", prevProjectionMatrix);
			model_cube.draw(shader_velocity2FBO);

			prevViewMatrix = viewMatrix;
			prevProjectionMatrix = projectionMatrix;

			// blend
			frameIdx ^= 1;
			fbo_history[frameIdx].bind();
			fbo_history[frameIdx ^ 1].bindColorTex(0);
			fbo_current.bindColorTex(1);
			fbo_velocity.bindColorTex(2);
			shader_TAA.bind();
			shader_TAA.setFloat1("mixRatio", mixRatio);
			renderer.clearAllBit();
			screenQuad.draw(shader_TAA);

			// show
			FrameBuffer::bindDefaultFrameBuffer();
			fbo_history[frameIdx].bindColorTex();
			screenQuad.draw(shader_screen);
		}

		#pragma region ImGui
		imguiManager.beginFrame("Paramater");

		ImGui::Checkbox("Freeze View", &freezeView);
		if (ImGui::RadioButton("No AntiAliasing", noAA))
		{
			useTAA = false;
			useMSAA = false;
			noAA = true;
		}
		if (ImGui::RadioButton("Use MSAA", useMSAA))
		{
			useTAA = false;
			useMSAA = true;
			noAA = false;
		}
		if (ImGui::RadioButton("Use TAA", useTAA))
		{
			useTAA = true;
			useMSAA = false;
			noAA = false;
		}
		if (useTAA)
		{
			ImGui::SliderFloat("Mix Ratio", &mixRatio, 0.05f, 1.0f);
		}

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
