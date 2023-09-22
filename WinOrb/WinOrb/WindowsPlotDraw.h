#ifndef WINDOWS_PLOT_DRAW
#define WINDOWS_PLOT_DRAW

#include "FFT.h"

//the idea is that I'll eventually make an abstract base class for drawing the FFT onto the Chosen One's Orb

//Imma slap this together real quick using  GLFW, Vulkan, and ImGui
class WindowsPlotDraw
{
public:

	bool Init();
	bool Update();
	void Destroy();

private:
	//glfw window manager
	//vulkan manager
	//imgui instance? I think I can just singleton it

};

#endif //!WINDOWS_PLOT_DRAW
