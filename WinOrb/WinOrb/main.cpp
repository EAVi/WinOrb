#include "WASAPILoopbackCapture.h"

int main()
{
	CoInitialize(NULL);

	WASAPILoopbackCapture device;
	Sleep(200);
	device.Capture();

	device.Destroy();
	return 0;
}