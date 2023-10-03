#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "GLFW/glfw3.h"

class WindowManager
{
public:
	static const int width = 640;
	static const int height = 480;
	bool Init();
	bool Update();
	bool Destroy();
	GLFWwindow* GetWindowPtr();
private:
	GLFWwindow* mWindow;
	bool mQuit;
};

#endif //!WINDOW_MANAGER_H