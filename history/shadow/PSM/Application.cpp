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
#define TEX_SIZE 4096
#define NEAR_PLANE 0.1f
#define FAR_PLANE 25.0f
#define HALF_FRUSTUM_SIZE 8.0f
#define PI 3.14159265359f

const float RADIUS = 4.2426f;
const float SPEED = 0.5f;

bool freezeView = false;
bool freezeTime = true;
enum ShadowOption
{
    SHADOW_NONE = 0,
    SHADOW_BASIC,
    SHADOW_PCF,
    SHADOW_PCSS
};
int shadowMode = SHADOW_NONE;
float filterScale = 1.0f;
float EPS = 0.04f;

string directory = {"F:/Visual Studio 2022/VisualStudioProject/LearnOpenGL/"};
inline string getPath(const string &relativePath)
{
    return directory + relativePath;
}

int main()
{
    Window window(WIDTH, HEIGHT, "还不如去玩原神");
    ImGuiManager imguiManager(window.getGLFWWindow());
    Model mCube(getPath("res/object/cube/cube.obj"));
    Model mRoom(getPath("res/object/shadow map/room.obj"));
    Model mCollection(getPath("res/object/shadow map/collection.obj"));
    ScreenQuad screenQuad;

    Shader sPrev(getPath("shader/preview.shader"));
    Shader sLight(getPath("shader/color.shader"));
    Shader sDrawShadowMap(getPath("shader/shadow/shadowMap/drawPointLightSM.vert"),
                          getPath("shader/shadow/shadowMap/drawPointLightSM.geom"),
                          getPath("shader/shadow/shadowMap/drawPointLightSM.frag"));
    Shader sUseShadowMap(getPath("shader/shadow/shadowMap/pointLightSM.vert"),
                         getPath("shader/shadow/shadowMap/pointLightSM.frag"));

    UniformBuffer ubo(2 * sizeof(mat4));

    PSM psm(TEX_SIZE, TEX_SIZE, NEAR_PLANE, FAR_PLANE);

    vec3 lightColor = vec3(1);
    vec3 lightPos = vec3(3, 5, -3);
    lightPos = vec3(0);
    float lightSize = 0.2f;

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

        if (!freezeTime)
        {
            time += window.getDeltaTime();
            float angle = SPEED * time - PI / 4.0f;
            lightPos.x = RADIUS * cosf(angle);
            lightPos.z = RADIUS * sinf(angle);
        }
        psm.updateLight(lightPos);

        // light pass
        psm.fbo.bind();
        renderer.setViewport(TEX_SIZE, TEX_SIZE);
        renderer.clearAllBit();

        sDrawShadowMap.bind();
        for (int i = 0; i < 6; ++i)
        {
            sDrawShadowMap.setMat4("lightPV[" + std::to_string(i) + "]", psm.lightPV[i]);
        }
        sDrawShadowMap.setMat4("model", modelMatrix);
        sDrawShadowMap.setFloat1("far_plane", FAR_PLANE);
        sDrawShadowMap.setVec3("lightPos", lightPos);
        mRoom.draw(sDrawShadowMap);
        mCollection.draw(sDrawShadowMap);

        // object pass
        psm.fbo.unbind();
        psm.fbo.bindDepthCubemap(2);
        renderer.setViewport(WIDTH, HEIGHT);
        renderer.clearAllBit();

        sUseShadowMap.bind();
        sUseShadowMap.bindBlock(ubo.getUBO(), "Matrices", 0);
        sUseShadowMap.setMat4("model", modelMatrix);
        sUseShadowMap.setInt1("shadowCubeMap", 2);
        sUseShadowMap.setInt1("shadowMode", shadowMode);
        sUseShadowMap.setFloat1("near_plane", NEAR_PLANE);
        sUseShadowMap.setFloat1("far_plane", FAR_PLANE);
        sUseShadowMap.setFloat1("texelSize", 1.0 / TEX_SIZE);
        sUseShadowMap.setFloat1("lightSizeUV", lightSize / (HALF_FRUSTUM_SIZE * 2));
        sUseShadowMap.setFloat1("filterScale", filterScale);
        sUseShadowMap.setFloat1("EPS", EPS);
        sUseShadowMap.setVec3("light.color", lightColor);
        sUseShadowMap.setVec3("light.pos", lightPos);
        sUseShadowMap.setVec3("light.ac", vec3(1.0, 0.045, 0.0075));
        sUseShadowMap.setVec3("viewPos", camera.position);
        mRoom.draw(sUseShadowMap);
        mCollection.draw(sUseShadowMap);

        modelMatrix = mat4(1);
        modelMatrix = translate(modelMatrix, lightPos);
        modelMatrix = scale(modelMatrix, 0.2f * vec3(1));
        sLight.bind();
        sLight.bindBlock(ubo.getUBO(), "Matrices", 0);
        sLight.setMat4("model", modelMatrix);
        sLight.setVec3("iColor", lightColor);
        mCube.draw(sLight);

#pragma region ImGui
        imguiManager.beginFrame("Paramater");

        ImGui::Checkbox("Freeze View", &freezeView);
        ImGui::Checkbox("Freeze Time", &freezeTime);
        ImGui::Text("Shadow Options");
        ImGui::RadioButton("No Shadow", &shadowMode, SHADOW_NONE);
        ImGui::RadioButton("Basic", &shadowMode, SHADOW_BASIC);
        ImGui::RadioButton("PCF", &shadowMode, SHADOW_PCF);
        ImGui::RadioButton("PCSS", &shadowMode, SHADOW_PCSS);
        ImGui::SliderFloat("filterScale", &filterScale, 0.0f, 2.0f);
        ImGui::SliderFloat("EPS", &EPS, 0.0f, 0.1f);

        imguiManager.endFrame();
#pragma endregion

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
