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

#define WIDTH  1200
#define HEIGHT 1200
#define PI 3.14159265359f

string directory = {
	"F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/" };
inline string getPath(const string& relativePath)
{
	return directory + relativePath;
}

enum { NEAREST = 0, BILINEAR, BICUBIC, BOX, ORIGIN };
int interpolationMethod = NEAREST;

int main()
{
	Window window(WIDTH, HEIGHT);
	ImGuiManager imguiManager(window.getGLFWWindow());

	Texture tInput("res/texture/test/uv.jpg");
	float scale = 4;
	vec2 outputSize = vec2(tInput.width, tInput.height) * scale;
	ImageTexture tOutput(outputSize.x, outputSize.y, GL_RGBA8, GL_NEAREST);
	tOutput.createImageHandle(GL_RGBA8, GL_WRITE_ONLY);

	Shader sCompute(getPath("shader/compute/interpolation_sampling.comp"), ShaderType::Compute);
	sCompute.bind();
	sCompute.set("outputSize", outputSize);
	sCompute.set("inputTexture", tInput.getHandle());
	sCompute.set("outputImage", tOutput.getImageHandle());

	ScreenQuad screenQuad;
	Shader sScreen(getPath("shader/screen.shader"));

	Renderer renderer;
	renderer.setClearColor(0.1, 0.1, 0.1, 1);
	renderer.setViewport(outputSize / scale);
	window.resize(outputSize / scale);

	while (!window.shouldClose())
	{
		renderer.clearAllBit();

		sCompute.bind();
		sCompute.set("interpolationMethod", interpolationMethod);
		float local_size = 16.0;
		glDispatchCompute(ceil(outputSize.x / local_size), ceil(outputSize.y / local_size), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		sScreen.bind();
		if (interpolationMethod == ORIGIN)
			sScreen.set("screenTexture", tInput.getHandle());
		else
			sScreen.set("screenTexture", tOutput.getHandle());
		screenQuad.draw(sScreen);

		imguiManager.beginFrame("select interpolation method");
		ImGui::RadioButton("Nearest", &interpolationMethod, NEAREST);
		ImGui::RadioButton("Bilinear", &interpolationMethod, BILINEAR); // 模糊, 适合下采样
		ImGui::RadioButton("Bicubic", &interpolationMethod, BICUBIC); // 锐化, 适合上采样
		ImGui::RadioButton("Box", &interpolationMethod, BOX);
		ImGui::RadioButton("Origin", &interpolationMethod, ORIGIN);
		imguiManager.endFrame();

		window.swapBuffers();
		window.pollEvents();
	}

	//tOutput.saveToJPG("");

	return 0;
}
