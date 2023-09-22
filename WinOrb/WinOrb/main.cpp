#include "WASAPILoopbackCapture.h"
#include "FFT.h"

int main()
{
	CoInitialize(NULL);

	WASAPILoopbackCapture device;
	constexpr unsigned totaltime = 32;
	constexpr unsigned numslices = 1;
	constexpr unsigned substep = totaltime / numslices;
	for (int i = 0; i < numslices; i++)
	{
		Sleep(substep);
		device.Capture();
	}
	
	complex_sample samples = device.GetSample();
	complex_sample frequency = FFT(samples);
	complex_sample modifiedsamples = IFFT(frequency);

	size_t peak_index = 0;
	for (int i = 0; i < frequency.size(); ++i)
	{
		if (std::abs(frequency[i]) > std::abs(frequency[peak_index]))
			peak_index = i;
	}
	float peak_freq = (float)device.SampleRate() / frequency.size() * peak_index;
	printf("frequency peaked at %f", peak_freq);

	device.Destroy();
	return 0;
}