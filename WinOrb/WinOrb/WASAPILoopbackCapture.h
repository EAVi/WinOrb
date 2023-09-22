#ifndef WASAPI_LOOPBACK_CAPTURE
#define WASAPI_LOOPBACK_CAPTURE

#include <mmdeviceapi.h>
#include <AudioClient.h>
#include <AudioPolicy.h>
#include "FFT.h"

class WASAPILoopbackCapture
{
public:
	static const size_t kSampleSize = 2048;
	static const size_t kFullSampleSize = kSampleSize * 2; // sample on two channels
	WASAPILoopbackCapture();
	bool Init();
	bool Capture();
	bool Destroy();
	complex_sample GetSample(bool leftchannel = false);
	unsigned SampleRate() { return mpwfx->nSamplesPerSec; };
private:
	IMMDeviceEnumerator* mpEnumerator = nullptr;
	IMMDevice* mpDevice = nullptr;
	IAudioClient* mpAudioClient = nullptr;
	IAudioCaptureClient* mpCaptureClient = nullptr;
	WAVEFORMATEX* mpwfx = nullptr;
	REFERENCE_TIME mhnsActualDuration = 0;
	float mSample[kFullSampleSize];
	size_t mSamplesCollected = 0;
};

#endif // WASAPI_LOOPBACK_CAPTURE