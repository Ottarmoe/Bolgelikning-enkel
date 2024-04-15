#pragma once

#include <iostream>
#include <windows.h>
#include <mmeapi.h>
#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <thread>
#include <vector>

#include "myvecs.h"


static double GLOBALFREQ = 440;
// Function to get the next sample of a sine wave
int16_t getNextPhaseSample() {
    const double sampleRate = 44100.0;  // Adjust as needed
    static double phase = 0;
    if(phase > 2*M_PI) phase -= 2*M_PI;

    phase += GLOBALFREQ*2*M_PI/double(sampleRate);
    return int16_t(sin(phase)*double(INT16_MAX/4));
}
void CALLBACK waveOutCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

class AudioStream{
private:
	bool shouldClose = 0;
	uint sampleRate;
	uint sampleBlockSize;

	WAVEFORMATEX wfx;
	HWAVEOUT waveOut;

	uint buffsize;
	std::vector<int16_t> buff1, buff2;
	WAVEHDR waveHdr1, waveHdr2;

	bool openConfigured(){
		//construct format tag
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = 1;              // Mono audio
		wfx.nSamplesPerSec = sampleRate;
		wfx.wBitsPerSample = 16;        // 16-bit samples
		wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
		wfx.cbSize = 0;
		//open stream
		if (waveOutOpen(&waveOut, WAVE_MAPPER, &wfx, reinterpret_cast<DWORD_PTR>(waveOutCallback), (DWORD_PTR)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
			std::cerr << "Error opening audio device." << std::endl;
			return 1;
    	}
		//prepare buffers
		waveHdr1 = WAVEHDR{0};
		waveHdr2 = WAVEHDR{0};
		waveHdr1.lpData = LPSTR(buff1.data());
    	waveHdr1.dwBufferLength = sampleBlockSize * sizeof(int16_t);
		waveHdr2.lpData = LPSTR(buff2.data());
    	waveHdr2.dwBufferLength = sampleBlockSize * sizeof(int16_t);
		//queue buffers
		waveOutPrepareHeader(waveOut, &waveHdr1, sizeof(WAVEHDR));
		waveOutPrepareHeader(waveOut, &waveHdr2, sizeof(WAVEHDR));
		waveOutWrite(waveOut, &waveHdr1, sizeof(WAVEHDR));
    	waveOutWrite(waveOut, &waveHdr2, sizeof(WAVEHDR));
		return 0;
	}
	void closeStream(){
		shouldClose = 1;
		waveOutUnprepareHeader(waveOut, &waveHdr1, sizeof(WAVEHDR));
    	waveOutUnprepareHeader(waveOut, &waveHdr2, sizeof(WAVEHDR));
		waveOutReset(waveOut);
		std::cout<<"cought exception on close\n";
		auto check = waveOutClose(waveOut);
		if (check != MMSYSERR_NOERROR) std::cout << "stream closing not good\n";
		else std::cout << "stream closing good\n";
	}

public:

	friend void CALLBACK waveOutCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

	//returns 1 for error
	bool reconfigure(){
		closeStream();
		return openConfigured();
	}
	AudioStream(uint sampleRate = 44100, uint blockSize = 2000)
		:sampleRate(sampleRate), sampleBlockSize(blockSize),
		buffsize(blockSize*3), buff1(buffsize, 0), buff2(buffsize, 0)
	{
		openConfigured();
	}
	~AudioStream(){
		closeStream();
	}
};


void CALLBACK waveOutCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    AudioStream* strm = (AudioStream*)dwInstance;
	if(strm->shouldClose) return;
	
	if(uMsg == WOM_CLOSE)
		std::cout<<"WOM closed\n";
	
	if(uMsg != WOM_DONE)
		return;
    
    // Retrieve the audio buffer information
    WAVEHDR* pWaveHdr = reinterpret_cast<WAVEHDR*>(dwParam1);
    int16_t* pBuffer = reinterpret_cast<int16_t*>(pWaveHdr->lpData);

    // Release the audio buffer
    waveOutUnprepareHeader(hwo, pWaveHdr, sizeof(WAVEHDR));

    for (DWORD i = 0; i < pWaveHdr->dwBufferLength / sizeof(int16_t); ++i) {
        pBuffer[i] = getNextPhaseSample();
    }

    // Prepare the buffer for playback
    waveOutPrepareHeader(hwo, pWaveHdr, sizeof(WAVEHDR));

    // Queue the buffer for playback
    waveOutWrite(hwo, pWaveHdr, sizeof(WAVEHDR));
}