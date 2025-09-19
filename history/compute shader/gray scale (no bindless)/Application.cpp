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

int main()
{
	Window window(WIDTH, HEIGHT);

	Texture inputTexture("D:/Pictures/Saved Pictures/cas.jpg");
	int w = inputTexture.width, h = inputTexture.height;
	Texture outputTexture(w, h);

	Shader sCompute(getPath("shader/compute/grayScale.comp"), ShaderType::Compute);
	sCompute.bind();
	inputTexture.bindImage(0, GL_READ_ONLY);
	outputTexture.bindImage(1, GL_WRITE_ONLY);

	float local_size = 16.0;
	glDispatchCompute(ceil(w / local_size), ceil(h / local_size), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	outputTexture.saveToJPG("D:/Pictures/Saved Pictures/cas gray.jpg");

	ScreenQuad screenQuad;
	Shader sScreen(getPath("shader/screen.shader"));
	sScreen.bind();
	sScreen.setInt1("screenTexture", 0);
	outputTexture.bind(0);

	Renderer renderer;
	renderer.setClearColor(0.1, 0.1, 0.1, 1);
	renderer.setViewport(w, h);
	window.resize(w, h);

	while (!window.shouldClose())
	{
		renderer.clearAllBit();

		screenQuad.draw(sScreen);

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
