#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h> 
#include "GLCall.h" // glew before glfw
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

class Camera;

class Window
{
public:
	Window(int width, int height, glm::vec3 cameraPos = glm::vec3(0.0f, 0.1f, 10.0f));
	//Window(int width, int height, const char* title, glm::vec3 cameraPos = glm::vec3(0.0f, 0.1f, 5.0f));
	~Window();

	bool shouldClose() const;
	void swapBuffers() const;
	void pollEvents() const;
	void processInput();
	void resize(int w, int h);
	void resize(vec2 v);

	GLFWwindow* getGLFWWindow() const { return m_window; }
	float getAspectRatio() const { return static_cast<float>(m_width) / m_height; }
	Camera& getCamera() { return *m_camera; }

	float getDeltaTime() const { return m_deltaTime; }
	float getTime();
	void updateTime();

	bool freezeView = false;

private:
	void initCallbacks();
	void initCamera(glm::vec3 pos);

	void handleKey(int key, int action);
	void handleMouse(double xpos, double ypos);
	void handleScroll(double yoffset);

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

	GLFWwindow* m_window;
	int m_width;
	int m_height;

	Camera* m_camera;

	double m_lastX;
	double m_lastY;
	bool m_firstMouse;
	bool m_canChangeView;

	float m_deltaTime;
	float m_lastFrame;
};