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
#include "StorageBuffer.h"

using namespace glm;
using namespace std;

#define WIDTH  1200
#define HEIGHT 1200
#define PI 3.14159265359f

bool freezeView = false;
bool useBloom = true;
bool useHDR = true;
bool enableTest = false;
float bloomThreshold = 0.8;
float bloomIntensity = 0.5;
int blurTimes = 3;

string directory = {
	"F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/" };
inline string getPath(const string& relativePath)
{
	return directory + relativePath;
}

int main()
{
	Window window(WIDTH, HEIGHT, "还不如去玩原神", vec3(0, 0, 3));
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model mCorridor(getPath("res/object/corridor/corridor.obj"));
	Model mCorridor0(getPath("res/object/corridor/corridor0.obj"));
	Model mLight(getPath("res/object/cube/cube.fbx"));
	vector<vec3> lightPos = {
		vec3(0.0f, 0.0f, -52.0f),
		vec3(-1.4f, -1.9f,-9.0f),
		vec3(0.0f, -1.8f, -4.0f),
		vec3(0.8f, -1.7f, -6.0f)
	};
	vector<vec3> lightColor = {
		1.f * vec3(1.0f, 1.0f, 0.0f),
		11.f * vec3(0.1f, 0.0f, 0.0f),
		11.f * vec3(0.0f, 0.0f, 0.2f),
		11.f * vec3(0.0f, 0.1f, 0.0f)
	};
	ScreenQuad screenQuad;

	Shader sPrev(getPath("shader/preview.shader"));
	Shader sLight(getPath("shader/color.shader"));
	Shader sPhong(getPath("shader/phong/corridor.shader"));
	Shader sShowOnScreen(getPath("shader/showOnScreen.shader"));
	Shader sBloomBlur(
		getPath("shader/postFX/postFX.vert"),
		getPath("shader/postFX/bloom/postFX_bloomBlur.frag"));
	Shader sBloomBlend(
		getPath("shader/postFX/postFX.vert"),
		getPath("shader/postFX/bloom/postFX_bloomBlend.frag"));

	UniformBuffer ubo(2 * sizeof(mat4));

	FrameBuffer sceneFBO(WIDTH, HEIGHT, FrameBufferType::COLOR_FLOAT_AND_RENDER, 2);
	int dsRatio = 4;
	FrameBuffer blurFBO[2]{
		FrameBuffer(WIDTH / dsRatio, HEIGHT / dsRatio, FrameBufferType::COLOR_FLOAT),
		FrameBuffer(WIDTH / dsRatio, HEIGHT / dsRatio, FrameBufferType::COLOR_FLOAT) };
	FrameBuffer upsampleFBO(WIDTH, HEIGHT, FrameBufferType::COLOR_FLOAT);

	Renderer renderer;
	//renderer.setClearColor(0.1, 0.1, 0.1, 1);
	renderer.setClearColor(0, 0, 0, 1);

	while (!window.shouldClose())
	{
		FrameBuffer::bindDefault();
		renderer.clearAllBit();

		window.updateTime();
		window.processInput();
		window.freezeView = freezeView;
		Camera camera = window.getCamera();

		mat4 modelMatrix(1);
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = camera.getProjectionMatrix();
		ubo.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
		ubo.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));

		// scene pass
		sceneFBO.bind();
		renderer.clearAllBit();
		renderer.enableDepthTest();
		{
			// corridor
			modelMatrix = mat4(1);
			sPhong.bind();
			sPhong.bindBlockUBO(ubo.getUBO(), 0);
			sPhong.setMat4("model", modelMatrix);
			sPhong.setVec3("viewPos", camera.position);
			sPhong.setFloat1("bloomThreshold", bloomThreshold);
			for (size_t i = 0; i < lightPos.size(); ++i)
			{
				std::string lightPosName = "lightPos[" + std::to_string(i) + "]";
				std::string lightColorName = "lightColor[" + std::to_string(i) + "]";
				sPhong.setVec3(lightPosName, lightPos[i]);
				sPhong.setVec3(lightColorName, lightColor[i]);
			}
			mCorridor.draw(sPhong);

			// light
			for (size_t i = 0; i < lightColor.size(); ++i)
			{
				modelMatrix = mat4(1);
				modelMatrix = translate(modelMatrix, lightPos[i]);
				if (i == 0) modelMatrix = scale(modelMatrix, 2.0f * vec3(1));
				else modelMatrix = scale(modelMatrix, 0.1f * vec3(1));
				sLight.bind();
				sLight.bindBlockUBO(ubo.getUBO(), 0);
				sLight.setMat4("model", modelMatrix);
				sLight.setVec3("iColor", lightColor[i]);
				mLight.draw(sLight);
			}
		}

		// down sample pass
		bool isHorizontal = true;
		blurFBO[isHorizontal].blitColorFrom(sceneFBO, 1);

		// blur pass
		renderer.setViewport(WIDTH / dsRatio, HEIGHT / dsRatio);
		sBloomBlur.bind();
		sBloomBlur.setInt1("screenTexture", 0);
		bool isFirstFrame = true;
		for (int i = 0; i < 2 * blurTimes; i++)
		{
			blurFBO[!isHorizontal].bind();
			blurFBO[isHorizontal].bindColorTex();
			//renderer.clearAllBit();
			sBloomBlur.setInt1("isHorizontal", !isHorizontal);
			screenQuad.draw(sBloomBlur);

			isHorizontal = !isHorizontal;
		}

		// up sample pass
		upsampleFBO.blitColorFrom(blurFBO[isHorizontal]);

		// blend pass
		FrameBuffer::bindDefault();
		renderer.clearAllBit();
		renderer.setViewport(WIDTH, HEIGHT);
		sceneFBO.bindColorTex(0, 0);
		upsampleFBO.bindColorTex(1, 0);
		sBloomBlend.bind();
		sBloomBlend.setInt1("sceneTexture", 0);
		sBloomBlend.setInt1("bloomTexture", 1);
		sBloomBlend.setInt1("useBloom", useBloom);
		sBloomBlend.setInt1("useHDR", useHDR);
		sBloomBlend.setFloat1("bloomIntensity", bloomIntensity);
		screenQuad.draw(sBloomBlend);				//最终混合结果

		// test
		if (enableTest)
		{
			FrameBuffer::bindDefault();
			renderer.clearAllBit();
			//sceneFBO.bindColorTex();				//原场景
			sceneFBO.bindColorTex(0, 1);			//高亮部分
			//blurFBO[isHorizontal].bindColorTex();	//降采样后的高亮模糊结果
			//upsampleFBO.bindColorTex();			//升采样后的高亮模糊结果
			sShowOnScreen.bind();
			sShowOnScreen.setInt1("screenTexture", 0);
			screenQuad.draw(sShowOnScreen);
		}

		#pragma region ImGui
		imguiManager.beginFrame("Parameter");

		ImGui::Checkbox("Freeze View", &freezeView);
		ImGui::Checkbox("Use Bloom", &useBloom);
		ImGui::Checkbox("Use HDR", &useHDR);
		ImGui::Checkbox("Enable Test", &enableTest);
		ImGui::SliderFloat("Bloom Threshold", &bloomThreshold, 0.0f, 1.0f);
		ImGui::SliderFloat("Bloom Intensity", &bloomIntensity, 0.0f, 1.0f);
		ImGui::SliderInt("Bloom Size", &blurTimes, 1, 10);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
