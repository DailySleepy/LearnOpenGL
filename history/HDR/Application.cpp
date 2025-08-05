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
bool useHDR = true;
float exposure = 1.0;

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
		vec3(0.0f, 0.0f, -50.0f),
		vec3(-1.4f, -1.9f,-9.0f),
		vec3(0.0f, -1.8f, -4.0f),
		vec3(0.8f, -1.7f, -6.0f)
	};
	vector<vec3> lightColor = {
		10.f * vec3(1.0f, 1.0f, 0.9f),
		1.0f * vec3(0.1f, 0.0f, 0.0f),
		1.0f * vec3(0.0f, 0.0f, 0.2f),
		1.0f * vec3(0.0f, 0.1f, 0.0f)
	};
	ScreenQuad screenQuad;

	Shader sPrev(getPath("shader/preview.shader"));
	Shader sLight(getPath("shader/color.shader"));
	Shader sPhong(getPath("shader/phong/corridor.shader"));
	Shader sHDR(
		getPath("shader/postFX/postFX.vert"),
		getPath("shader/postFX/image enhancement/postFX_HDR.frag"));

	UniformBuffer ubo(2 * sizeof(mat4));

	FrameBuffer fbo(WIDTH, HEIGHT, FrameBufferType::COLOR_FLOAT_AND_RENDER);

	Renderer renderer;
	renderer.setClearColor(0.1, 0.1, 0.1, 1);

	while (!window.shouldClose())
	{
		fbo.bind();
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

		// corridor
		modelMatrix = mat4(1);
		sPhong.bind();
		sPhong.bindBlockUBO(ubo.getUBO(), 0);
		sPhong.setMat4("model", modelMatrix);
		sPhong.setVec3("viewPos", camera.position);
		for (size_t i = 0; i < lightPos.size(); ++i)
		{
			std::string lightPosName = "lightPos[" + std::to_string(i) + "]";
			std::string lightColorName = "lightColor[" + std::to_string(i) + "]";
			sPhong.setVec3(lightPosName, lightPos[i]);
			sPhong.setVec3(lightColorName, lightColor[i]);
		}
		mCorridor.draw(sPhong);

		sPhong.setVec3("lightPos[0]", lightPos[0] + vec3(0, 0, 5));
		mCorridor0.draw(sPhong);

		// light
		/*for (size_t i = 0; i < lightColor.size(); ++i)
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
		}*/

		fbo.unbind();
		fbo.bindColorTex();
		renderer.clearAllBit();
		sHDR.bind();
		sHDR.setInt1("screenTexture", 0);
		sHDR.setInt1("useHDR", useHDR);
		sHDR.setFloat1("exposure", exposure);
		screenQuad.draw(sHDR);

		#pragma region ImGui
		imguiManager.beginFrame("Parameter");

		ImGui::Checkbox("Freeze View", &freezeView);
		ImGui::Checkbox("Use HDR", &useHDR);
		ImGui::SliderFloat("Exposure", &exposure, 0, 10);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
