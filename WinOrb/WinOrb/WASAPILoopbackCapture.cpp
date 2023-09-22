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
template<typename T, int size>
void LShiftArray(T (&a)[size], size_t numshifts)
{
	if (numshifts == 0)
		return;
	if (numshifts >= size)
	{
		memset(&a[0], 0, sizeof(a));
		return;
	}

	size_t keep = size - numshifts; // amount we're keeping from the original array
	memmove(&a[0], &a[numshifts], sizeof(T) * keep);
	memset(&a[keep], 0, numshifts);
}

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

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

	mhnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / mpwfx->nSamplesPerSec;
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
		else //copy data to buffer
		{
			constexpr size_t asize = ARRAY_SIZE(mSample);
			size_t adjustedLen = packetLength * mpwfx->nChannels;
			LShiftArray(mSample, adjustedLen);
			size_t writestart = asize - adjustedLen;
			if (writestart < 0)
			{
				writestart = 0;
				adjustedLen = asize;
			}
			memcpy(&mSample[writestart], (void*)pData, adjustedLen * sizeof(float));
			mSamplesCollected += packetLength;
		}

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

complex_sample WASAPILoopbackCapture::GetSample(bool leftchannel)
{
	complex_sample sample;
	//channels are interlaced between eachother on the buffer,
	// eg [l, r, l, r, l, r, ...] 
	//naturally, we only care about one channel at a time
	int numchannels = mpwfx->nChannels;
	int start = leftchannel ? 0 : (numchannels - 1);
	
	if (numchannels <= 0 || mSamplesCollected == 0)
		return sample;

	for (int i = start, dstindex = 0; i < kFullSampleSize; i += numchannels, ++dstindex)
	{
		sample.emplace_back(mSample[i], 0.0f);
	}
	return sample;
}