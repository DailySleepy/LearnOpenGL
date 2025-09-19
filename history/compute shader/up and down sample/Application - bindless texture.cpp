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
float scaleFactor = 1;
float lastScaleFactor = scaleFactor;
bool syncViewportWithWindowSize = false;

int main()
{
	Window window(WIDTH, HEIGHT);
	ImGuiManager imguiManager(window.getGLFWWindow());

	Texture tInput("res/texture/test/uv.jpg");
	vec2 inputSize = vec2(tInput.getWidth(), tInput.getHeight());
	vec2 outputSize = inputSize * scaleFactor;
	ImageTexture tOutput(outputSize.x, outputSize.y, GL_WRITE_ONLY, GL_RGBA8, GL_RGBA8, GL_NEAREST);

	Shader sCompute(getPath("shader/compute/interpolation_sampling.comp"), ShaderType::Compute);
	sCompute.bind();
	sCompute.set("inputTexture", tInput.getHandle());
	sCompute.set("outputSize", outputSize);
	sCompute.set("outputImage", tOutput.getImageHandle());

	ScreenQuad screenQuad;
	Shader sScreen(getPath("shader/screen.shader"));

	Renderer renderer;
	renderer.setClearColor(0.1, 0.1, 0.1, 1);
	renderer.setViewport(outputSize);
	window.resize(outputSize / scaleFactor);

	while (!window.shouldClose())
	{
		renderer.clearAllBit();

		if (scaleFactor != lastScaleFactor)
		{
			outputSize = inputSize * scaleFactor;
			tOutput = ImageTexture(outputSize.x, outputSize.y, GL_WRITE_ONLY, GL_RGBA8, GL_RGBA8, GL_NEAREST);

			sCompute.bind();
			sCompute.set("outputSize", outputSize);
			sCompute.set("outputImage", tOutput.getImageHandle());

			renderer.setViewport(syncViewportWithWindowSize ? outputSize / scaleFactor : outputSize);
			window.resize(outputSize / scaleFactor);

			lastScaleFactor = scaleFactor;
		}

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

		imguiManager.beginFrame("Image Interpolation");
		ImGui::Checkbox("syncViewportWithWindowSize", &syncViewportWithWindowSize);
		ImGui::SliderFloat("Scale", &scaleFactor, 0.1f, 10.0f, "%.2f");
		ImGui::RadioButton("Nearest", &interpolationMethod, NEAREST);
		ImGui::RadioButton("Bilinear", &interpolationMethod, BILINEAR); // 模糊, 适合下采样
		ImGui::RadioButton("Bicubic", &interpolationMethod, BICUBIC); // 锐化, 适合上采样
		ImGui::RadioButton("Box", &interpolationMethod, BOX);
		ImGui::RadioButton("Origin", &interpolationMethod, ORIGIN);
		imguiManager.endFrame();

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
