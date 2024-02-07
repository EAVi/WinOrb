#include "WASAPILoopbackCapture.h"
#include "WindowManager.h"
#include "VulkanDoodler.h"
#include "FFT.h"
#include <assert.h>

int main()
{
	CoInitialize(NULL);

	WASAPILoopbackCapture device;
	VulkanDoodler doodler;
	device.Init();
	doodler.Init();
	while (!doodler.IsQuit())
	{
		Sleep(16);
		device.Capture();
		complex_sample samples = device.GetSample();
		complex_sample frequency = FFT(samples);
		std::vector<float> magnitudes = ToMagnitude(frequency);
		magnitudes.resize(1024);
		doodler.UpdateChart(magnitudes);
		doodler.Update();
	}
	
	device.Destroy();
	doodler.Destroy();
	return 0;
}