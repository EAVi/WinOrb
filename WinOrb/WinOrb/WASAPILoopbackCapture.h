#ifndef WASAPI_LOOPBACK_CAPTURE
#define WASAPI_LOOPBACK_CAPTURE

#include <mmdeviceapi.h>
#include <AudioClient.h>
#include <AudioPolicy.h>

class WASAPILoopbackCapture
{
public:
	WASAPILoopbackCapture();
	bool Init();
	bool Capture();
	bool Destroy();
private:
	IMMDeviceEnumerator* mpEnumerator = nullptr;
	IMMDevice* mpDevice = nullptr;
	IAudioClient* mpAudioClient = nullptr;
	IAudioCaptureClient* mpCaptureClient = nullptr;
	WAVEFORMATEX* mpwfx = nullptr;
	REFERENCE_TIME mhnsActualDuration = 0;
	static const size_t kSampleSize = 1024;
	float mSample[kSampleSize];
};

#endif // WASAPI_LOOPBACK_CAPTURE