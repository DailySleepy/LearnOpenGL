#include <glm.hpp>

#include <iostream>
#include <map>
#include <string>

#include "Camera.h"
#include "ImGuiManager.h"
#include "Model.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include "Window.h"

using namespace glm;
using namespace std;

#define WIDTH 1200
#define HEIGHT 1200

string directory = {"F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/"};
inline string getPath(const string &relativePath)
{
    return directory + relativePath;
}

int main()
{
    Window window(WIDTH, HEIGHT, "还不如去玩原神");
    ImGuiManager imguiManager(window.getGLFWWindow());

    Model mPlanet(getPath("res/object/planet/planet.obj"));
    Model mRock(getPath("res/object/rock/rock.obj"));
    vector<mat4> modelMatrices(10000);
    srand(static_cast<unsigned int>(window.getTime()));
    float r = 25.0f, offset = 12.5f;
    for (int i = 0; i < modelMatrices.size(); i++)
    {
        mat4 modelMatrix = mat4(1);

        float angle = (float)i / (float)modelMatrices.size() * 360.0f;
        float displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * r + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * r + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.12f;
        modelMatrix = translate(modelMatrix, vec3(x, y, z));

        float s = (rand() % 20) / 100.0f + 0.05f;
        modelMatrix = scale(modelMatrix, 0.5f * vec3(s));

        float rot = (float)(rand() % 360);
        modelMatrix = rotate(modelMatrix, rot, vec3(0.4f, 0.6f, 0.8f));

        modelMatrices[i] = modelMatrix;
    }
    VertexBufferLayout layout;
    layout.push<float>(4);
    layout.push<float>(4);
    layout.push<float>(4);
    layout.push<float>(4);
    mRock.addInstanceData(modelMatrices.data(), sizeof(modelMatrices[0]) * modelMatrices.size(), layout);

    Shader sPrev(getPath("shader/preview.shader"));
    Shader sPlanet(getPath("shader/instance/phong_planet.vert"), getPath("shader/instance/phong_planetAndRock.frag"));
    Shader sRock(getPath("shader/instance/phong_rock.vert"), getPath("shader/instance/phong_planetAndRock.frag"));

    UniformBuffer ubo(2 * sizeof(mat4));

    Renderer renderer;
    renderer.setClearColor(0.05, 0.05, 0.05, 1);

    while (!window.shouldClose())
    {
        renderer.clearAllBit();
        renderer.d_CullFace();

        window.updateTime();
        window.processInput();
        Camera camera = window.getCamera();

        mat4 modelMatrix = mat4(1);
        mat4 viewMatrix = camera.getViewMatrix();
        mat4 projectionMatrix = perspective(radians(camera.fov), window.getAspectRatio(), 0.1f, 300.0f);

        ubo.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
        ubo.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));

        {
            sPlanet.bind();
            sPlanet.bindUBO(ubo.getUBO(), 0);
            sPlanet.setMat4("model", modelMatrix);
            sPlanet.setVec3("viewPos", camera.position);
            sPlanet.setVec3("light.color", vec3(1));
            sPlanet.setVec3("light.dir", vec3(0, 0, -1));
            mPlanet.draw(sPlanet);
        }
        {
            sRock.bind();
            sRock.bindUBO(ubo.getUBO(), 0);
            sRock.setVec3("viewPos", camera.position);
            sRock.setVec3("light.color", vec3(1));
            sRock.setVec3("light.dir", vec3(0, 0, -1));
            mRock.draw(sRock, modelMatrices.size());
        }

#pragma region ImGui
        imguiManager.beginFrame("Paramater");

        imguiManager.endFrame();
#pragma endregion

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}