#include "Window.h"
#include "Camera.h"
#include "stb_image.h"


void loadIcon(GLFWwindow* window)
{
	GLFWimage icon;
	int iconWidth, iconHeight, iconChannels;
	unsigned char* iconData = stbi_load("res/icon/icon.png", &iconWidth, &iconHeight, &iconChannels, 4);
	if (iconData)
	{
		icon.width = iconWidth;
		icon.height = iconHeight;
		icon.pixels = iconData;
		glfwSetWindowIcon(window, 1, &icon);

		HWND hwnd = glfwGetWin32Window(window);
		if (hwnd)
		{
			HICON hIcon = (HICON)LoadImageW(NULL, L"res/icon/icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
			if (hIcon)
			{
				SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

				SendMessageTimeout(
					FindWindow("Shell_TrayWnd", NULL),
					WM_SETTINGCHANGE,
					0,
					(LPARAM)"TraySettings",
					SMTO_ABORTIFHUNG,
					1000,
					NULL
				);
			}
		}
		stbi_image_free(iconData);
	}
}

void initRenderMode()
{
	//glEnable(GL_FRAMEBUFFER_SRGB);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_STENCIL_TEST);

	/*glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

Window::Window(int width, int height, glm::vec3 cameraPos)
	: m_width(width), m_height(height), m_lastX(width / 2.0), m_lastY(height / 2.0)
	, m_firstMouse(true), m_canChangeView(true), m_deltaTime(0.0f), m_lastFrame(0.0f)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_SAMPLES, 4); // use MSAA

	const char* title = "能玩一辈子图形学吗";
	m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwMakeContextCurrent(m_window);
	loadIcon(m_window);
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	initCallbacks();
	initCamera(cameraPos);

	glewInit();

	initRenderMode();
}

Window::~Window()
{
	delete m_camera;
	glfwTerminate();
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(m_window);
}

void Window::swapBuffers() const
{
	glfwSwapBuffers(m_window);
}

void Window::pollEvents() const
{
	glfwPollEvents();
}

void Window::processInput()
{
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
		m_camera->processKeyboard(FORWARD, m_deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
		m_camera->processKeyboard(BACKWARD, m_deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
		m_camera->processKeyboard(LEFT, m_deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
		m_camera->processKeyboard(RIGHT, m_deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
		m_camera->processKeyboard(UP, m_deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		m_camera->processKeyboard(DOWN, m_deltaTime);
}

void Window::resize(int w, int h)
{
	m_width = w, m_height = h;
	glfwSetWindowSize(getGLFWWindow(), w, h);
}

void Window::resize(vec2 v)
{
	resize(v.x, v.y);
}

float Window::getTime()
{
	return glfwGetTime();
}

void Window::updateTime()
{
	float currentFrame = glfwGetTime();
	m_deltaTime = currentFrame - m_lastFrame;
	m_lastFrame = currentFrame;
}

void Window::initCallbacks()
{
	glfwSetWindowUserPointer(m_window, this);

	glfwSetKeyCallback(m_window, keyCallback);
	glfwSetCursorPosCallback(m_window, mouseCallback);
	glfwSetScrollCallback(m_window, scrollCallback);
	glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
}

void Window::initCamera(glm::vec3 pos)
{
	m_camera = new Camera(pos);
	m_camera->setAspectRatio(static_cast<float>(m_width) / m_height);
}

void Window::handleKey(int key, int action)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, true);

	if (key == GLFW_KEY_LEFT_ALT)
	{
		if (action == GLFW_PRESS)
		{
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			m_canChangeView = false;
			m_firstMouse = true;
		}
		else if (action == GLFW_RELEASE)
		{
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			m_canChangeView = true;
		}
	}
}

void Window::handleMouse(double xpos, double ypos)
{
	if (!m_canChangeView || freezeView) return;

	if (m_firstMouse)
	{
		m_lastX = xpos;
		m_lastY = ypos;
		m_firstMouse = false;
	}

	float xoffset = xpos - m_lastX;
	float yoffset = m_lastY - ypos;  // 反转Y轴
	m_lastX = xpos;
	m_lastY = ypos;

	m_camera->processMouseMovement(xoffset, yoffset);
}

void Window::handleScroll(double yoffset)
{
	m_camera->processMouseScroll(yoffset);
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (win) win->handleKey(key, action);
}

void Window::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (win) win->handleMouse(xpos, ypos);
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (win) win->handleScroll(yoffset);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	//glViewport(0, 0, width, height);
}
