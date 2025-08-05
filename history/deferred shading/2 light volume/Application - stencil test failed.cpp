#include <glm.hpp>

#include <iostream>
#include <map>
#include <string>


#include "Camera.h"
#include "FrameBuffer.h"
#include "ImGuiManager.h"
#include "Model.h"
#include "ScreenQuad.h"
#include "Shader.h"
#include "StorageBuffer.h"
#include "UniformBuffer.h"
#include "Window.h"

using namespace glm;
using namespace std;

#define WIDTH 1200
#define HEIGHT 1200
#define PI 3.14159265359f

bool freezeView = false;
float R_max = 10.0;

string directory = {"F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/"};
inline string getPath(const string &relativePath)
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

int main()
{
    Window window(WIDTH, HEIGHT);
    ImGuiManager imguiManager(window.getGLFWWindow());

    Model mNanosuit(getPath("res/object/nanosuit/nanosuit.obj"));
    Model mLight(getPath("res/object/sphere/sphere.fbx"));

    const int lightCount = 1;
    vector<vec3> lightPos(lightCount);
    vector<vec3> lightColor(lightCount);
    vector<pair<vec3, vec3>> lightData;
    lightData.reserve(lightCount);
    for (int i = 0; i < lightCount; i++)
    {
        lightPos[i] = randomPoint(10);
        lightColor[i] = randomColor(0.2);
        lightData.emplace_back(lightPos[i], lightColor[i]);
    }
    VertexBufferLayout layout;
    layout.push<float>(3);
    layout.push<float>(3);
    mLight.addInstanceData(lightData.data(), lightData.size() * sizeof(lightData[0]), layout);

    ScreenQuad screenQuad;

    Shader sPrev(getPath("shader/preview.shader"));
    Shader sScreen(getPath("shader/screen.shader"));
    Shader sDrawLight(getPath("shader/deferred/instancedLight.shader"));
    Shader sRenderGbuffer(getPath("shader/deferred/renderGbuffer.vert"), getPath("shader/deferred/renderGbuffer.frag"));
    Shader sUseGbuffer(getPath("shader/deferred/useGbuffer.vert"), getPath("shader/deferred/useGbuffer.frag"));
    Shader sLightVolume(getPath("shader/deferred/lightVolume.vert"), getPath("shader/deferred/lightVolume.frag"));
    Shader sUpdateStencil(getPath("shader/deferred/updateStencil.shader"));

    UniformBuffer ubo(2 * sizeof(mat4));

    FrameBuffer gBuffer(WIDTH, HEIGHT, FrameBufferType::G_BUFFER);

    Renderer renderer;
    renderer.setClearColor(vec3(0.1), 1.0);
    renderer.setClearColor(vec3(0), 1.0);

    while (!window.shouldClose())
    {
        window.updateTime();
        window.processInput();
        window.freezeView = freezeView;
        Camera camera = window.getCamera();

        mat4 modelMatrix(1);
        mat4 viewMatrix = camera.getViewMatrix();
        mat4 projectionMatrix = camera.getProjectionMatrix();
        ubo.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
        ubo.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));

        // render G buffer
        {
            gBuffer.bind();
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

        // use G buffer
        {
            FrameBuffer::bindDefault();
            renderer.clearAllBit();
            renderer.d_DepthWrite();

            // stencil pass
            if (true)
            {
                FrameBuffer::blit(GL_DEPTH_BUFFER_BIT, gBuffer.getFBO(), WIDTH, HEIGHT, 0, WIDTH, HEIGHT);
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_ALWAYS, 0, 0xff);
                glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
                glStencilMask(0xff);
                renderer.d_ColorWrite();

                sUpdateStencil.bind();
                sUpdateStencil.bindUBO(ubo.getUBO(), 0);
                sUpdateStencil.setFloat1("R_max", R_max);

                renderer.e_CullFaceBack();
                renderer.e_DepthTestLess();
                mLight.draw(sUpdateStencil, lightCount);

                renderer.e_CullFaceFront();
                renderer.e_DepthTestGreater();
                mLight.draw(sUpdateStencil, lightCount);

                glStencilFunc(GL_EQUAL, 2, 0xff);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                renderer.e_ColorWrite();

                sScreen.bind();
                renderer.e_CullFaceBack();
                renderer.e_DepthTestLess();
                screenQuad.draw(sScreen);
            }
            renderer.e_BlendAdd();

            gBuffer.bindColorTex(0, 0);
            gBuffer.bindColorTex(1, 1);
            gBuffer.bindColorTex(2, 2);
            sLightVolume.bind();
            sLightVolume.bindUBO(ubo.getUBO(), 0);
            sLightVolume.setInt1("gPosition", 0);
            sLightVolume.setInt1("gNormal", 1);
            sLightVolume.setInt1("gAlbedoSpec", 2);
            sLightVolume.setVec3("viewPos", camera.position);
            sLightVolume.setFloat1("R_max", R_max);

            renderer.e_CullFaceFront();
            mLight.draw(sLightVolume, lightCount);
            renderer.e_CullFaceBack();
            renderer.e_DepthTestLess();

            renderer.d_Blend();
            renderer.e_DepthWrite();
        }

        // draw lights
        {
            glDisable(GL_STENCIL_TEST);
            FrameBuffer::blit(GL_DEPTH_BUFFER_BIT, gBuffer.getFBO(), WIDTH, HEIGHT, 0, WIDTH, HEIGHT);

            sDrawLight.bind();
            sDrawLight.bindUBO(ubo.getUBO(), 0);
            sDrawLight.setFloat1("radius", 0.05f);
            mLight.draw(sDrawLight, lightCount);
        }

#pragma region ImGui
        imguiManager.beginFrame("Parameter");

        ImGui::Checkbox("Freeze View", &freezeView);
        ImGui::SliderFloat("Max Atten Radius", &R_max, 0, 10);

        imguiManager.endFrame();
#pragma endregion

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
