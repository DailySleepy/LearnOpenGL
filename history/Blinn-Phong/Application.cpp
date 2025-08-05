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

bool freezeView = false;

int main()
{
	Window window(WIDTH, HEIGHT, "还不如去玩原神");
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model mCube("res/object/cube/cube.fbx");
	Model mFloor("res/object/ground/wood floor/floor.obj");
	//Model mKfk("res/object/kfk/kfk.obj");

	Shader sPrev("shader/preview.shader");
	Shader sLight("shader/light.shader");
	Shader sPhong("shader/phong.vert", "shader/phong.frag");

	UniformBuffer ubo(2 * sizeof(mat4));

	vec3 lightColor = vec3(1);
	vec3 lightPos = vec3(2, 3, -2);
	float lightStength = 1.5;
	bool blinn = false;
	struct Factor
	{
		float ambient = 1.0f;
		float diffuse = 1.0f;
		float specular = 1.0f;
	};
	Factor factor;
	float shininess = 32;

	Renderer renderer;
	renderer.setClearColor(0.05, 0.05, 0.05, 1);


	while (!window.shouldClose())
	{
		renderer.clearAllBit();

		window.updateTime();
		window.processInput();
		window.freezeView = freezeView;
		Camera camera = window.getCamera();

		mat4 modelMatrix = mat4(1);
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = perspective(radians(camera.fov), window.getAspectRatio(), 0.1f, 300.0f);

		ubo.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
		ubo.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));

		{
			ubo.bindPoint(sLight, "Matrices", 0);
			sLight.bind();
			modelMatrix = mat4(1);
			modelMatrix = translate(modelMatrix, lightPos);
			modelMatrix = scale(modelMatrix, 0.2f * vec3(1));
			sLight.setMat4("model", modelMatrix);
			sLight.setVec3("iColor", lightColor);
			mCube.draw(sLight);
		}
		{
			ubo.bindPoint(sPhong, "Matrices", 0);
			sPhong.bind();
			modelMatrix = mat4(1);
			modelMatrix = translate(modelMatrix, vec3(0, -1, 0));
			modelMatrix = scale(modelMatrix, 10.0f * vec3(1));
			sPhong.setMat4("model", modelMatrix);
			sPhong.setVec3("light.color", lightColor * lightStength);
			sPhong.setVec3("light.pos", lightPos);
			sPhong.setVec3("light.ac", vec3(1.0, 0.045, 0.0075));
			sPhong.setVec3("viewPos", camera.position);
			sPhong.setInt1("blinn", blinn);
			sPhong.setFloat1("factor.ambient", factor.ambient);
			sPhong.setFloat1("factor.diffuse", factor.diffuse);
			sPhong.setFloat1("factor.specular", factor.specular);
			sPhong.setFloat1("material.shininess", shininess);
			mFloor.draw(sPhong);
		}

		#pragma region ImGui
		imguiManager.beginFrame("Paramater");

		ImGui::Checkbox("Freeze View", &freezeView);
		ImGui::Checkbox("blinn", &blinn);
		ImGui::SliderFloat("Light Stength", &lightStength, 0.0f, 2.0f);
		ImGui::SliderFloat("Shininess", &shininess, 0, 1024);
		ImGui::SliderFloat("Ambient", &factor.ambient, 0.0f, 2.0f);
		ImGui::SliderFloat("Diffuse", &factor.diffuse, 0.0f, 2.0f);
		ImGui::SliderFloat("Specular", &factor.specular, 0.0f, 2.0f);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
