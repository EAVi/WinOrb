#ifndef WINDOWS_PLOT_DRAW
#define WINDOWS_PLOT_DRAW

//think this file is useless, consider deleting

#include "FFT.h"
#include "WindowManager.h"
#include "WASAPILoopbackCapture.h"
#include "VulkanDoodler.h"

//the idea is that I'll eventually make an abstract base class for drawing the FFT onto the Chosen One's Orb

//Imma slap this together real quick using  GLFW, Vulkan, and ImGui
class WindowsPlotDraw
{
public:

	bool Init();
	bool Update();
	void Destroy();

private:
	WASAPILoopbackCapture mCapture;
	WindowManager mWindow;
	VulkanDoodler mDoodler;
};

#endif //!WINDOWS_PLOT_DRAW
