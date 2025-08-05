#include <string>
#include <map>

#include "Camera.h"
#include "FrameBuffer.h"
#include "ImGuiManager.h"
#include "Model.h"
#include "ScreenQuad.h"
#include "Shader.h"
#include "StorageBuffer.h"
#include "UniformBuffer.h"
#include "Window.h"

#define WIDTH 1200
#define HEIGHT 1200
#define TILE_SIZE 16
#define PI 3.14159265359f

const int lightCount = 1024;
const int maxLightsPerTile = 256;
const int tileCountX = ceil(WIDTH / TILE_SIZE);
const int tileCountY = ceil(HEIGHT / TILE_SIZE);
const int tileCount = tileCountX * tileCountY;

bool freezeView = false;
bool useAABBtest = true;
float R_max = 4.0;

string directory = { "F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/" };
inline string getPath(const string& relativePath)
{
	return directory + relativePath;
}

vec3 randomPoint(float radius)
{
	float radius2 = radius * 2;
	float x = static_cast<float>(rand()) / RAND_MAX * radius2 - radius;
	float y = static_cast<float>(rand()) / RAND_MAX * radius2 - radius;
	float z = static_cast<float>(rand()) / RAND_MAX * radius2 - radius;
	return vec3(x, y, z);
}
vec3 randomColor(float minVal)
{
	float r, g, b;
	do
	{
		r = static_cast<float>(rand()) / RAND_MAX;
		g = static_cast<float>(rand()) / RAND_MAX * 0.85;
		b = static_cast<float>(rand()) / RAND_MAX;
	} while (r < minVal && g < minVal && b < minVal);
	return vec3(r, g, b);
}
pair<vector<vec4>, vector<vec4>> getLightData(int lightCount)
{
	vector<vec4> lightPos(lightCount), lightColor(lightCount);

	for (int i = 0; i < lightCount; i++)
	{
		lightPos[i] = vec4(randomPoint(10), 1);
		lightColor[i] = vec4(randomColor(0.2), 1);
	}
	return make_pair(lightPos, lightColor);
}

int main()
{
	Window window(WIDTH, HEIGHT);
	ImGuiManager imguiManager(window.getGLFWWindow());

	Model mNanosuit(getPath("res/object/nanosuit/nanosuit.obj"));
	Model mLight(getPath("res/object/sphere/sphere.fbx"));

	ScreenQuad screenQuad;

	#pragma region Shaders
	Shader sPrev(getPath("shader/preview.shader"));
	Shader sScreen(getPath("shader/screen.shader"));
	Shader sDrawLight(getPath("shader/deferred/instancedLight.shader"));
	Shader sRenderGbuffer(
		getPath("shader/deferred/renderGbuffer.vert"),
		getPath("shader/deferred/renderGbuffer.frag"));
	Shader sDepthRange(
		getPath("shader/deferred/tiled/depthViewRange.comp"), ShaderType::Compute);
	Shader sTransformLight(
		getPath("shader/deferred/transformLight.comp"), ShaderType::Compute);
	Shader sTileLight(
		getPath("shader/deferred/tiled/tileLight_zViewRange.comp"), ShaderType::Compute);
	Shader sTiledDeferred(
		getPath("shader/deferred/tiled/tiledDeferred.vert"),
		getPath("shader/deferred/tiled/tiledDeferred.frag"));
	#pragma endregion

	int lightDataSize = lightCount * sizeof(vec4);
	auto [lightPos, lightColor] = getLightData(lightCount);
	#pragma region addInstanceData
	VertexBufferLayout layout; layout.push<float>(4);
	mLight.addInstanceData(lightPos.data(), lightDataSize, layout);
	mLight.addInstanceData(lightColor.data(), lightDataSize, layout);
	#pragma endregion
	StorageBuffer ssbo_lightPos(lightDataSize, lightPos.data());
	StorageBuffer ssbo_lightPosView(lightDataSize);
	StorageBuffer ssbo_lightColor(lightDataSize, lightColor.data());
	StorageBuffer ssbo_lightIndices(tileCount * maxLightsPerTile * sizeof(uint32_t));
	StorageBuffer ssbo_depthViewRangeInt(tileCount * sizeof(ivec2));
	StorageBuffer ssbo_debug(tileCount * sizeof(vec2));
	StorageBuffer ssbo_debug1(tileCount * sizeof(int));
	//vector<ivec2> depthInit(tileCount, ivec2(floatBitsToInt(1.0f), floatBitsToInt(-1.0f))); // NDC
	//vector<ivec2> depthInit(tileCount, ivec2(floatBitsToInt(FLT_MAX), floatBitsToInt(-FLT_MAX))); // view 
	// floatBitsToInt(-FLT_MAX) = -8388609, int depthInt = floatBitsToInt(depth) = -1073455104, compare failed!
	vector<ivec2> depthInit(tileCount, ivec2(floatBitsToInt(FLT_MAX), -2073455104));

	ImageTexture debugImage(WIDTH, HEIGHT, GL_RGBA16F);

	UniformBuffer ubo(2 * sizeof(mat4));

	FrameBuffer G_Buffer(WIDTH, HEIGHT, FrameBufferType::G_BUFFER);
	printf("G_Buffer.getDepthTextureHandle(): %llx\n", G_Buffer.getDepthTextureHandle());

	Renderer renderer;
	renderer.setClearColor(vec3(0), 0);

	while (!window.shouldClose())
	{
		window.updateTime();
		window.processInput();
		window.freezeView = freezeView;
		Camera camera = window.getCamera();

		mat4 modelMatrix(1);
		mat4 viewMatrix = camera.getViewMatrix();
		mat4 projectionMatrix = camera.getProjectionMatrix();
		mat4 invProj = inverse(projectionMatrix);
		ubo.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
		ubo.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));


		// render G buffer
		{
			G_Buffer.bind();
			renderer.clearAllBit();

			modelMatrix = mat4(1);
			sRenderGbuffer.bind();
			sRenderGbuffer.bindUBO(ubo.getUBO(), 0);
			for (int i = -1; i <= 1; ++i)
			{
				for (int j = -1; j <= 1; ++j)
				{
					modelMatrix = mat4(1.0f);
					modelMatrix = translate(modelMatrix, vec3(i * 3.0f, -1.0f, j * 3.0f));
					modelMatrix = scale(modelMatrix, 0.2f * vec3(1));
					sRenderGbuffer.setMat4("model", modelMatrix);
					mNanosuit.draw(sRenderGbuffer);
				}
			}
		}

		// tile lights - compute shader
		{
			// compute tile z min and max
			//G_Buffer.bindColorTex(0, 0); // position
			sDepthRange.bind();
			sDepthRange.bindUBO(ubo.getUBO(), 0);
			ssbo_depthViewRangeInt.setSubData(0, tileCount * sizeof(ivec2), depthInit.data());
			sDepthRange.bindSSBO(ssbo_depthViewRangeInt.getSSBO(), 0);
			sDepthRange.bindSSBO(ssbo_debug1.getSSBO(), 1);
			//sDepthRange.setInt1("gPosition", 0);
			sDepthRange.setHandle("gPosition", G_Buffer.getColorTextureHandle(0));
			sDepthRange.setInt1("tileSize", TILE_SIZE);
			sDepthRange.setInt1("tileCountX", tileCountX);
			sDepthRange.setFloat2("screenSize", WIDTH, HEIGHT);
			glDispatchCompute(tileCountX, tileCountY, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			// compute lightPosView
			sTransformLight.bind();
			sTransformLight.bindUBO(ubo.getUBO(), 0);
			sTransformLight.bindSSBO(ssbo_lightPos.getSSBO(), 0);
			sTransformLight.bindSSBO(ssbo_lightPosView.getSSBO(), 1);
			sTransformLight.setInt1("lightCount", lightCount);
			glDispatchCompute(ceil(float(lightCount) / 64), 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			// compute tile light list
			sTileLight.bind();
			sTileLight.bindSSBO(ssbo_lightPosView.getSSBO(), 0);
			sTileLight.bindSSBO(ssbo_lightIndices.getSSBO(), 1);
			sTileLight.bindSSBO(ssbo_depthViewRangeInt.getSSBO(), 2);
			sTileLight.bindSSBO(ssbo_debug.getSSBO(), 3);
			debugImage.bindImage(0, GL_WRITE_ONLY, GL_RGBA16F);
			sTileLight.setMat4("invProj", invProj);
			sTileLight.setInt1("lightCount", lightCount);
			sTileLight.setInt1("tileSize", TILE_SIZE);
			sTileLight.setInt1("tileCountX", tileCountX);
			sTileLight.setInt1("tileCountY", tileCountY);
			sTileLight.setInt1("maxLightsPerTile", maxLightsPerTile);
			sTileLight.setFloat2("screenSize", WIDTH, HEIGHT);
			sTileLight.setFloat1("R_max", R_max);
			sTileLight.setInt1("useAABBtest", useAABBtest);
			glDispatchCompute(ceil(float(tileCount) / 64), 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}

		// render tiles
		{
			FrameBuffer::bindDefault();
			renderer.clearAllBit();
			renderer.d_DepthWrite();

			G_Buffer.bindColorTex(0, 0); // position
			G_Buffer.bindColorTex(1, 1); // normal
			G_Buffer.bindColorTex(2, 2); // albedo - spec
			sTiledDeferred.bind();
			sTiledDeferred.bindUBO(ubo.getUBO(), 0);
			sTiledDeferred.bindSSBO(ssbo_lightPos.getSSBO(), 0);
			sTiledDeferred.bindSSBO(ssbo_lightColor.getSSBO(), 1);
			sTiledDeferred.bindSSBO(ssbo_lightIndices.getSSBO(), 2);
			sTiledDeferred.setInt1("gPosition", 0);
			sTiledDeferred.setInt1("gNormal", 1);
			sTiledDeferred.setInt1("gAlbedoSpec", 2);
			sTiledDeferred.setInt1("tileSize", TILE_SIZE);
			sTiledDeferred.setInt1("tileCountX", tileCountX);
			sTiledDeferred.setInt1("maxLightsPerTile", maxLightsPerTile);
			sTiledDeferred.setVec3("viewPos", camera.position);
			sTiledDeferred.setFloat1("R_max", R_max);
			screenQuad.draw(sTiledDeferred);

			renderer.e_DepthWrite();
		}

		// draw lights
		{
			FrameBuffer::blit(GL_DEPTH_BUFFER_BIT, G_Buffer.getFBO(), WIDTH, HEIGHT, 0, WIDTH, HEIGHT);

			sDrawLight.bind();
			sDrawLight.bindUBO(ubo.getUBO(), 0);
			sDrawLight.setFloat1("radius", 0.05f);
			mLight.draw(sDrawLight, lightCount);
		}

	renderingOver:
		#pragma region ImGui
		imguiManager.beginFrame("Parameter");

		ImGui::Checkbox("Freeze View", &freezeView);
		ImGui::Checkbox("Use AABB Test", &useAABBtest);
		ImGui::SliderFloat("Max Atten Radius", &R_max, 0, 10);

		imguiManager.endFrame();
		#pragma endregion

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
