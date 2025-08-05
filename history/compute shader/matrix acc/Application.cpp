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

string directory = {
	"F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/" };
inline string getPath(const string& relativePath)
{
	return directory + relativePath;
}

int main()
{
	Window window(WIDTH, HEIGHT, "还不如去玩原神");

	const int size = 2000;
	const int m(size), k(size), n(size);

	vector<float> matrixA(m * k, 1.0f);
	vector<float> matrixB(k * n, 1.0f);
	vector<float> matrixC(m * n, 0.0f);

	StorageBuffer ssboA(matrixA.size() * sizeof(float), matrixA.data());
	StorageBuffer ssboB(matrixB.size() * sizeof(float), matrixB.data());
	StorageBuffer ssboC(matrixC.size() * sizeof(float), matrixC.data());

	Shader sCompute(getPath("shader/compute/matrix.comp"), ShaderType::Compute);
	sCompute.bind();
	sCompute.setInt1("m", m);
	sCompute.setInt1("k", k);
	sCompute.setInt1("n", n);
	sCompute.bindBlockSSBO(ssboA.getSSBO(), 0);
	sCompute.bindBlockSSBO(ssboB.getSSBO(), 1);
	sCompute.bindBlockSSBO(ssboC.getSSBO(), 2);

	GLuint queryID;
	glGenQueries(1, &queryID);
	glBeginQuery(GL_TIME_ELAPSED, queryID);

	const float local_size = 16.0f;
	glDispatchCompute(ceil(m / local_size), ceil(n / local_size), 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glEndQuery(GL_TIME_ELAPSED);
	GLuint64 elapsedTime;
	glGetQueryObjectui64v(queryID, GL_QUERY_RESULT, &elapsedTime);
	float timeMs = elapsedTime / 1e6f;
	std::cout << "GPU Compute Time: " << timeMs << " ms" << std::endl;

	ssboC.getData(matrixC.size() * sizeof(float), matrixC.data());

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			printf("%f ", matrixC[i * n + j]);
		}
		printf("\n");
	}

	return 0;
}
