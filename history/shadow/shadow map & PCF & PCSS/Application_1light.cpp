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
const float RADIUS = 4.2426f;
const float SPEED1 = 0.5f;
const float SPEED2 = 5.0f;
const float PI = 3.14159265359f;

bool freezeView = false;
enum ShadowOption
{
	SHADOW_NONE = 0,
	SHADOW_BASIC,
	SHADOW_PCF,
	SHADOW_PCSS
};
int shadowMode = SHADOW_NONE;
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
	Shader sPhong("shader/phong_shadowMap.vert", "shader/phong_shadowMap.frag");
	Shader sDrawShadowMap("shader/shadowMap_draw.shader");
	Shader sShowShadowMap("shader/shadowMap_show.shader");
	sShowShadowMap.bind();
	sShowShadowMap.setInt1("shadowMap", 0);

	UniformBuffer ubo(2 * sizeof(mat4));
	UniformBuffer ubo_lightVP(2 * sizeof(mat4));

	FrameBuffer fbo_shadowMap(TEX_SIZE, TEX_SIZE, DEPTH);

	vec3 lightColor = vec3(1);
	vec3 lightPos = vec3(3, 5, -3);
	vec3 lightFoucs = vec3(0, 1, 0);
	float lightSize = 0.2f;

	Renderer renderer;
	renderer.setClearColor(0.05, 0.05, 0.05, 1);

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

		mat4 lightViewMatrix = lookAt(lightPos, lightFoucs, vec3(0, 1, 0));
		mat4 lightProjectionMatrix = ortho(-HALF_FRUSTUM_SIZE, HALF_FRUSTUM_SIZE, -HALF_FRUSTUM_SIZE, HALF_FRUSTUM_SIZE, 0.1f, 12.0f);
		ubo_lightVP.setSubData(0, sizeof(mat4), value_ptr(lightProjectionMatrix));
		ubo_lightVP.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(lightViewMatrix));

		fbo_shadowMap.bind();
		renderer.setViewport(TEX_SIZE, TEX_SIZE);
		renderer.clearAllBit();
		{
			sDrawShadowMap.bind();
			sDrawShadowMap.bindBlock(ubo_lightVP.getUBO(), "LightMatrices", 0);
			sDrawShadowMap.setMat4("model", modelMatrix);
			//renderer.cullFrontFace();
			mBall.draw(sDrawShadowMap);
			mCube.draw(sDrawShadowMap);
			//renderer.cullBackFace();
		}

		fbo_shadowMap.unbind();
		fbo_shadowMap.bindDepthTex(2);
		renderer.setViewport(WIDTH, HEIGHT);
		renderer.clearAllBit();
		float angle = SPEED1 * window.getTime() - PI / 4.0f;
		lightPos.x = RADIUS * cosf(angle);
		lightPos.z = RADIUS * sinf(angle);
		{
			sPhong.bind();
			sPhong.bindBlock(ubo.getUBO(), "Matrices", 0);
			sPhong.bindBlock(ubo_lightVP.getUBO(), "LightMatrices", 1);
			sPhong.setInt1("shadowMap", 2);
			sPhong.setFloat1("texelSize", 1.0 / TEX_SIZE);
			sPhong.setFloat1("lightSizeUV", lightSize / (HALF_FRUSTUM_SIZE * 2));
			sPhong.setFloat1("filterScale", filterScale);
			sPhong.setInt1("shadowMode", shadowMode);
			sPhong.setMat4("model", modelMatrix);
			sPhong.setVec3("light.color", lightColor);
			sPhong.setVec3("light.dir", lightFoucs - lightPos);
			sPhong.setVec3("viewPos", camera.position);
			mBall.draw(sPhong);
			mCube.draw(sPhong);
		}
		{
			modelMatrix = mat4(1);
			modelMatrix = translate(modelMatrix, lightPos);
			modelMatrix = scale(modelMatrix, 0.2f * vec3(1));
			sLight.bind();
			sLight.bindBlock(ubo.getUBO(), "Matrices", 0);
			sLight.setMat4("model", modelMatrix);
			sLight.setVec3("iColor", lightColor);
			mCube.draw(sLight);
		}

		#pragma region ImGui
		imguiManager.beginFrame("Paramater");

		ImGui::Checkbox("Freeze View", &freezeView);
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