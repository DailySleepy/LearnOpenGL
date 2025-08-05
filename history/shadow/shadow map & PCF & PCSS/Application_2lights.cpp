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
#define TEX_SIZE 2048
#define HALF_FRUSTUM_SIZE 8.0f
#define PI 3.14159265359f

bool freezeView = false;
bool freezeTime = false;
enum ShadowOption
{
	SHADOW_NONE = 0,
	SHADOW_BASIC,
	SHADOW_PCF,
	SHADOW_PCSS
};
int shadowMode = SHADOW_PCSS;
float filterScale = 1.0f;

int main()
{
	Window window(WIDTH, HEIGHT, "还不如去玩原神");
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model mCube("res/object/cube/cube.fbx");
	Model mBall("res/object/shadow map/ball.obj");
	ScreenQuad screenQuad;
	//Model mKfk("res/object/kfk/kfk.obj");

	Shader sPrev("shader/preview.shader");
	Shader sLight("shader/light.shader");
	Shader sPhong("shader/phong_shadowMap_2lights.vert", "shader/phong_shadowMap_2lights.frag");
	Shader sDrawShadowMap("shader/shadowMap_draw.shader");
	Shader sShowShadowMap("shader/shadowMap_show.shader");
	sShowShadowMap.bind();
	sShowShadowMap.setInt1("shadowMap", 0);

	UniformBuffer ubo(2 * sizeof(mat4));
	UniformBuffer ubo_lightVP[2] = { UniformBuffer(2 * sizeof(mat4)), UniformBuffer(2 * sizeof(mat4)) };

	FrameBuffer fbo_shadowMap[2] = { FrameBuffer(TEX_SIZE, TEX_SIZE, DEPTH), FrameBuffer(TEX_SIZE, TEX_SIZE, DEPTH) };

	vec3 lightColor[2] = { vec3(1), vec3(0.8, 0.8, 1.0) };
	vec3 lightPos[2] = { vec3(3, 5, -3), vec3(3, 5, -3) };
	vec3 lightFoucs[2] = { vec3(0, 1, 0), vec3(0, 1, 0) };
	float lightSize[2] = { 0.2f, 0.3f };
	float speed[2] = { 0.5f, 2.5f };
	float r = 4.2426f;

	Renderer renderer;
	renderer.setClearColor(0.05, 0.05, 0.05, 1);

	float time = window.getTime();

	while (!window.shouldClose())
	{
		window.updateTime();
		window.processInput();
		window.freezeView = freezeView;
		Camera camera = window.getCamera();

		mat4 modelMatrix = mat4(1);
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = perspective(radians(camera.fov), window.getAspectRatio(), 0.1f, 300.0f);
		ubo.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
		ubo.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));

		if (!freezeTime)
		{
			time += window.getDeltaTime();
			mat4 lightProjectionMatrix = ortho(-HALF_FRUSTUM_SIZE, HALF_FRUSTUM_SIZE, -HALF_FRUSTUM_SIZE, HALF_FRUSTUM_SIZE, 0.1f, 12.0f);
			for (int i = 0; i < 2; i++)
			{
				float angle = speed[i] * time - PI / 4.0f;
				lightPos[i].x = r * cosf(angle);
				lightPos[i].z = r * sinf(angle);
				mat4 lightViewMatrix = lookAt(lightPos[i], lightFoucs[i], vec3(0, 1, 0));
				ubo_lightVP[i].setSubData(0, sizeof(mat4), value_ptr(lightProjectionMatrix));
				ubo_lightVP[i].setSubData(sizeof(mat4), sizeof(mat4), value_ptr(lightViewMatrix));
			}
		}

		// light pass
		{
			renderer.setViewport(TEX_SIZE, TEX_SIZE);
			for (int i = 0; i < 2; i++)
			{
				fbo_shadowMap[i].bind();
				renderer.clearAllBit();
				{
					sDrawShadowMap.bind();
					sDrawShadowMap.bindBlock(ubo_lightVP[i].getUBO(), "LightMatrices", i);
					sDrawShadowMap.setMat4("model", modelMatrix);
					mBall.draw(sDrawShadowMap);
					mCube.draw(sDrawShadowMap);
				}
			}
		}
		// object pass
		{
			FrameBuffer::bindDefaultFrameBuffer();
			fbo_shadowMap[0].bindDepthTex(2);
			fbo_shadowMap[1].bindDepthTex(3);
			renderer.setViewport(WIDTH, HEIGHT);
			renderer.clearAllBit();
			{
				sPhong.bind();
				sPhong.bindBlock(ubo.getUBO(), "Matrices", 0);
				sPhong.bindBlock(ubo_lightVP[0].getUBO(), "LightMatrices[0]", 1);
				sPhong.bindBlock(ubo_lightVP[1].getUBO(), "LightMatrices[1]", 2);
				sPhong.setInt1("shadowMap[0]", 2);
				sPhong.setInt1("shadowMap[1]", 3);
				sPhong.setFloat1("texelSize", 1.0 / TEX_SIZE);
				sPhong.setFloat1("lightSizeUV[0]", lightSize[0] / (HALF_FRUSTUM_SIZE * 2));
				sPhong.setFloat1("lightSizeUV[1]", lightSize[1] / (HALF_FRUSTUM_SIZE * 2));
				sPhong.setFloat1("filterScale", filterScale);
				sPhong.setInt1("shadowMode", shadowMode);
				sPhong.setMat4("model", modelMatrix);
				sPhong.setVec3("light[0].color", lightColor[0]);
				sPhong.setVec3("light[0].dir", lightFoucs[0] - lightPos[0]);
				sPhong.setVec3("light[1].color", lightColor[1]);
				sPhong.setVec3("light[1].dir", lightFoucs[1] - lightPos[1]);
				sPhong.setVec3("viewPos", camera.position);
				mBall.draw(sPhong);
				mCube.draw(sPhong);
			}
			{
				for (int i = 0; i < 2; i++)
				{
					modelMatrix = mat4(1);
					modelMatrix = translate(modelMatrix, lightPos[i]);
					modelMatrix = scale(modelMatrix, lightSize[i] * vec3(1));
					sLight.bind();
					sLight.bindBlock(ubo.getUBO(), "Matrices", 0);
					sLight.setMat4("model", modelMatrix);
					sLight.setVec3("iColor", lightColor[i]);
					mCube.draw(sLight);
				}
			}
		}

		#pragma region ImGui
		imguiManager.beginFrame("Paramater");

		ImGui::Checkbox("Freeze View", &freezeView);
		ImGui::Checkbox("Freeze Time", &freezeTime);
		ImGui::Text("Shadow Options");
		ImGui::RadioButton("No Shadow", &shadowMode, SHADOW_NONE);
		ImGui::RadioButton("Basic", &shadowMode, SHADOW_BASIC);
		ImGui::RadioButton("PCF", &shadowMode, SHADOW_PCF);
		ImGui::RadioButton("PCSS", &shadowMode, SHADOW_PCSS);
		ImGui::SliderFloat("filterScale", &filterScale, 0.0f, 2.0f);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
