#include <glm.hpp>

#include <iostream>
#include <map>
#include <string>

#include "Camera.h"
#include "FrameBuffer.h"
#include "ImGuiManager.h"
#include "Model.h"
#include "PSM.h"
#include "ScreenQuad.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include "Window.h"

using namespace glm;
using namespace std;

#define WIDTH 1200
#define HEIGHT 1200
#define PI 3.14159265359f

bool freezeView = false;
float xoffset = 0.0;

string directory = {"F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/"};
inline string getPath(const string &relativePath)
{
    return directory + relativePath;
}

int main()
{
    Window window(WIDTH, HEIGHT, "还不如去玩原神", vec3(0, 0, 20));
    ImGuiManager imguiManager(window.getGLFWWindow());

    Model mCube(getPath("res/object/cube/cube.obj"));
    Model mFloor(getPath("res/object/ground/wood floor/floor.obj"));

    Shader sPrev(getPath("shader/preview.shader"));
    Shader sLight(getPath("shader/color.shader"));
    Shader sPhong(getPath("shader/phong/basic.shader"));
    Shader sPlanarShadow(getPath("shader/shadow/planar shadow/planarShadow.vert"),
                         getPath("shader/shadow/planar shadow/planarShadow.frag"));

    UniformBuffer ubo(2 * sizeof(mat4));

    vec3 lightPos = vec3(2, 5, 2);
    vec3 lightColor = vec3(1);
    float lightStrength = 1.5f;
    float lightSize = 0.2f;

    vec3 planeN = vec3(0, 1, 0);
    float planeD = 5.0f;

    Renderer renderer;
    renderer.setClearColor(0.1, 0.1, 0.1, 1);

    float time = window.getTime();

    while (!window.shouldClose())
    {
        renderer.clearAllBit();

        window.updateTime();
        window.processInput();
        window.freezeView = freezeView;
        Camera camera = window.getCamera();

        mat4 modelMatrix = mat4(1);
        mat4 viewMatrix = camera.getViewMatrix();
        mat4 projectionMatrix = perspective(radians(camera.fov), window.getAspectRatio(), 0.1f, 100.0f);
        ubo.setSubData(0, sizeof(mat4), value_ptr(projectionMatrix));
        ubo.setSubData(sizeof(mat4), sizeof(mat4), value_ptr(viewMatrix));

        // floor
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xff);
        vec3 offset = vec3(xoffset, 0, 0);
        modelMatrix = mat4(1);
        modelMatrix = translate(modelMatrix, planeN * -planeD + offset);
        modelMatrix = scale(modelMatrix, 10.0f * vec3(1));
        sPrev.bind();
        sPrev.bindBlockUBO(ubo.getUBO(), "Matrices", 0);
        sPrev.setMat4("model", modelMatrix);
        mFloor.draw(sPrev);

        // cube shadow
        glStencilFunc(GL_EQUAL, 1, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
        glStencilMask(0xff);
        glDisable(GL_DEPTH_TEST);
        modelMatrix = mat4(1);
        sPlanarShadow.bind();
        sPlanarShadow.bindBlockUBO(ubo.getUBO(), "Matrices", 0);
        sPlanarShadow.setMat4("model", modelMatrix);
        sPlanarShadow.setVec3("lightPos", lightPos);
        sPlanarShadow.setVec4("plane", vec4(planeN, planeD));
        mCube.draw(sPlanarShadow);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        // cube
        modelMatrix = mat4(1);
        sPhong.bind();
        sPhong.bindBlockUBO(ubo.getUBO(), "Matrices", 0);
        sPhong.setMat4("model", modelMatrix);
        sPhong.setVec3("viewPos", camera.position);
        sPhong.setVec3("light.color", lightColor * lightStrength);
        sPhong.setVec3("light.pos", lightPos);
        mCube.draw(sPhong);

        // light
        modelMatrix = mat4(1);
        modelMatrix = translate(modelMatrix, lightPos);
        modelMatrix = scale(modelMatrix, lightSize * vec3(1));
        sLight.bind();
        sLight.bindBlockUBO(ubo.getUBO(), "Matrices", 0);
        sLight.setMat4("model", modelMatrix);
        sLight.setVec3("iColor", lightColor);
        mCube.draw(sLight);

#pragma region ImGui
        imguiManager.beginFrame("Paramater");

        ImGui::Checkbox("Freeze View", &freezeView);
        ImGui::SliderFloat("xoffset", &xoffset, -10, 10);

        imguiManager.endFrame();
#pragma endregion

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
