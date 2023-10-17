#include "WindowManager.h"
#include <cassert>

void WindowManager::Init()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	mWindow = glfwCreateWindow(width, height, "Purple's Orb of Pondering", nullptr, nullptr);
	mQuit = false;

	glfwSetWindowUserPointer(mWindow, this);
	glfwSetWindowSizeCallback(mWindow, FramebufferSizeCallback);

	assert(mWindow != nullptr);
}

void WindowManager::Update()
{
	assert(mWindow != nullptr);
	if (!glfwWindowShouldClose(mWindow))
	{
		glfwPollEvents();
	}
	else
	{
		mQuit = true;
	}
}

void WindowManager::Destroy()
{
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void WindowManager::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	auto windowuser = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
	if (windowuser != nullptr)
	{
		windowuser->mResize = true;
	}
}
