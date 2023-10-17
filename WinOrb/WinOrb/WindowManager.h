#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "GLFW/glfw3.h"

class WindowManager
{
public:
	static const int width = 640;
	static const int height = 480;
	virtual void Init();
	virtual void Update();
	virtual void Destroy();
	bool IsQuit() { return mQuit; };
	bool IsResize() { return mResize; };
	bool ResetResize() { mResize = false; };
protected:
	GLFWwindow* mWindow;
	bool mQuit;
	bool mResize;
	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);


};

#endif //!WINDOW_MANAGER_H