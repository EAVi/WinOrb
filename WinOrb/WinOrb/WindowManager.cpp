#include "WindowManager.h"

bool WindowManager::Init()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	mWindow = glfwCreateWindow(width, height, "Purple's Orb of Pondering", nullptr, nullptr);
	return mWindow != nullptr;
}

bool WindowManager::Update()
{
	if (mWindow == nullptr)
	{
		return false;
	}
	if (!glfwWindowShouldClose(mWindow))
	{
		glfwPollEvents();
		mQuit = true;
		return true;
	}
	return false;
}

bool WindowManager::Destroy()
{
	glfwDestroyWindow(mWindow);
	glfwTerminate();
	return true;
}

GLFWwindow* WindowManager::GetWindowPtr()
{
	return mWindow;
}
