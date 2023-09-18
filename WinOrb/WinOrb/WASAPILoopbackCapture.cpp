#include "WASAPILoopbackCapture.h"
#include <mmdeviceapi.h>
#include <AudioClient.h>
#include <AudioPolicy.h>

#define RETURN_ON_FAIL(hres) if(FAILED(hres)) return false;
#define RELEASE(punk) if(punk){punk->Release(); punk = nullptr; }

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

//shift an array to the left
template<typename T, size_t N>
void LShiftArray(T a[N], size_t numshifts)
{
	if (numshifts == 0)
		return;
	if (numshifts >= N)
	{
		memset(&a[0], 0, sizeof(a));
		return;
	}

	size_t keep = N - numshifts; // amount we're keeping from the original array
	memmove(&a[0], &a[numshifts], sizeof(T) * keep);
	memset(&a[keep], 0, numshifts);
}

WASAPILoopbackCapture::WASAPILoopbackCapture()
{
	memset(&mSample[0], 0, sizeof(mSample));
	Init();
}

bool WASAPILoopbackCapture::Init()
{
	HRESULT hr;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	UINT32 bufferFrameCount;
	UINT32 packetLength = 0;
	BOOL bDone = FALSE;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&mpEnumerator);
	RETURN_ON_FAIL(hr);

	hr = mpEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &mpDevice);
	RETURN_ON_FAIL(hr);

	hr = mpDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&mpAudioClient);
	RETURN_ON_FAIL(hr);

	hr = mpAudioClient->GetMixFormat(&mpwfx);
	RETURN_ON_FAIL(hr);

	hr = mpAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_LOOPBACK,
		hnsRequestedDuration,
		0,
		mpwfx,
		NULL);
	RETURN_ON_FAIL(hr);

	hr = mpAudioClient->GetBufferSize(&bufferFrameCount);
	RETURN_ON_FAIL(hr);

	hr = mpAudioClient->GetService(IID_IAudioCaptureClient, (void**)&mpCaptureClient);
	RETURN_ON_FAIL(hr);

	//audio sink will be passed into the capture function
	//any information about format will need to be stored
	//todo remove this commment when everything works

	mhnsActualDuration = REFTIMES_PER_SEC * bufferFrameCount / mpwfx->nSamplesPerSec;
	hr = mpAudioClient->Start();
	RETURN_ON_FAIL(hr);

	return true;
}

bool WASAPILoopbackCapture::Capture()
{
	HRESULT hr;
	UINT32 numFramesAvailable;
	UINT32 packetLength = 0;
	DWORD flags;
	BYTE* pData;

	hr = mpCaptureClient->GetNextPacketSize(&packetLength);
	RETURN_ON_FAIL(hr);

	while (packetLength != 0)
	{
		hr = mpCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
		RETURN_ON_FAIL(hr);

		if(flags & AUDCLNT_BUFFERFLAGS_SILENT)
		{
			pData = nullptr;
		}

		//copy pData here

		hr = mpCaptureClient->ReleaseBuffer(numFramesAvailable);
		RETURN_ON_FAIL(hr);

		hr = mpCaptureClient->GetNextPacketSize(&packetLength);
		RETURN_ON_FAIL(hr);
	}
	return false;
}

bool WASAPILoopbackCapture::Destroy()
{
	HRESULT hr;
	hr = mpAudioClient->Stop();  // Stop recording.
	RETURN_ON_FAIL(hr);

	CoTaskMemFree(mpwfx);
	RELEASE(mpEnumerator);
	RELEASE(mpDevice);
	RELEASE(mpAudioClient);
	RELEASE(mpCaptureClient);
	return true;
}
