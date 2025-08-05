#pragma once
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h" 
#include "imconfig.h"

struct GLFWwindow;

class ImGuiManager
{
public:
	ImGuiManager(ImGuiManager&) = delete;

	ImGuiManager(GLFWwindow* window, const char* glsl_version = "#version 460 core")
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		setFont();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init(glsl_version);
	}

	void setFont()
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.Fonts->Clear();
		io.Fonts->AddFontFromFileTTF("res/font/ARLRDBD.TTF", 18.0f);
		io.Fonts->Build();
	}

	~ImGuiManager()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void beginFrame(const char* title)
	{
		//glDisable(GL_FRAMEBUFFER_SRGB);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin(title);
	}

	void endFrame()
	{
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//glEnable(GL_FRAMEBUFFER_SRGB);
	}
};