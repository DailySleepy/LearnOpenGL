#include <glm.hpp>

#include <sstream>
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

string directory = { "F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/" };
inline string getPath(const string& relativePath)
{
	return directory + relativePath;
}

int main()
{
	Window window(WIDTH, HEIGHT);

	Texture tInput("D:/Pictures/Saved Pictures/cas1.jpg");
	tInput.createHandle();
	int w = tInput.width, h = tInput.height;

	ImageTexture tOutput(w, h, GL_RGBA8);
	tOutput.createHandle();
	tOutput.createImageHandle(GL_RGBA8, GL_WRITE_ONLY);

	Shader sCompute(getPath("shader/compute/grayScale - bindless texture.comp"), ShaderType::Compute);
	sCompute.bind();
	sCompute.setHandle("inputTexture", tInput.getHandle());
	sCompute.setHandle("outputImage", tOutput.getImageHandle());

	float local_size = 16.0;
	glDispatchCompute(ceil(w / local_size), ceil(h / local_size), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	//tOutput.saveToJPG("D:/Pictures/Saved Pictures/cas1 gray.jpg");

	ScreenQuad screenQuad;
	Shader sScreen(getPath("shader/screen.shader"));
	sScreen.bind();
	sScreen.setHandle("screenTexture", tOutput.getHandle());

	Renderer renderer;
	renderer.setClearColor(0.1, 0.1, 0.1, 1);
	renderer.setViewport(w / 2, h / 2);
	window.resize(w / 2, h / 2);

	while (!window.shouldClose())
	{
		renderer.clearAllBit();

		screenQuad.draw(sScreen);

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
