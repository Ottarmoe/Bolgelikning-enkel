#pragma once

#include <iostream>
#include <windows.h>


//for the windows multimedia waveform audio documentation, see
//https://learn.microsoft.com/en-us/windows/win32/multimedia/devices-and-data-types
//and
//https://learn.microsoft.com/en-us/windows/win32/multimedia/waveform-functions
#include <mmeapi.h>
#include <stdint.h>
#include <math.h>
#include <thread>
#include <vector>
//#include <mutex>

#include "myvecs.h"

void CALLBACK waveOutCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

//struct that is passed along with the audio buffer header
//in order to insure proper memory flow
struct audioBufferInfo{
	std::vector<int16_t>* boundVector;
};

//as the waveOutCallback function executes on a seperate thread
//the audio stream buffer vector needs to be thread-safe
template<typename T>
struct ThreadAdaptedVector{
	std::vector<T> vec;
	CRITICAL_SECTION cs;

	ThreadAdaptedVector(){
		InitializeCriticalSection(&cs);
	}
	~ThreadAdaptedVector(){
		DeleteCriticalSection(&cs);
	}
	void enter(){
		EnterCriticalSection(&cs);
	}
	void exit(){
		LeaveCriticalSection(&cs);
	}

};

class AudioStream{
private:
	bool shouldClose = 0; //communicates with waveOutCallback to close the stream
	uint sampleRate;
	uint sampleBlockSize;
	
	//because the simulation providing samples, and the audio driver, are not necessarily 
	//perfectly syncronized, the AudioStream provides a sample rate to the simulator
	//that is dynamically regulated.
	//if the last block had too few samples, the rate is sped up
	//if the last block had too many, it is slowed down
	//this is done by a P regulator
	double regSampleRate;
	double kp = 0.5;

	WAVEFORMATEX wfx;
	HWAVEOUT waveOut;

	//modified by callback functions, should not be modified by class when running
	std::vector<int16_t> buff1, buff2;
	WAVEHDR waveHdr1, waveHdr2;
	audioBufferInfo waveHdr1Inf, waveHdr2Inf;

	//std::mutex queWriteControl;
	ThreadAdaptedVector<int16_t> que;

	bool openConfigured(){
		shouldClose = 0;
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

		waveHdr1 = WAVEHDR{0, 0, 0, 0, 0, 0, 0, 0};
		waveHdr2 = WAVEHDR{0, 0, 0, 0, 0, 0, 0, 0};
		waveHdr1Inf.boundVector = &buff1;
		waveHdr2Inf.boundVector = &buff2;
		waveHdr1.lpData = LPSTR(buff1.data());
		waveHdr1.dwUser = DWORD_PTR(&waveHdr1Inf);
    	waveHdr1.dwBufferLength = sampleBlockSize * sizeof(int16_t);
		waveHdr2.lpData = LPSTR(buff2.data());
		waveHdr2.dwUser = DWORD_PTR(&waveHdr2Inf);
    	waveHdr2.dwBufferLength = sampleBlockSize * sizeof(int16_t);

		//queue buffers, starting the rolling buffer system
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
		auto check = waveOutClose(waveOut);
		if (check != MMSYSERR_NOERROR) std::cerr << "error on stream close\n";
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
		buff1(blockSize*3, 0), buff2(blockSize*3, 0)
	{
		openConfigured();
	}
	~AudioStream(){
		closeStream();
	}

	double getSampleRate() const {return regSampleRate;}
	double getInternalSampleRate() const {return sampleRate;}
	uint getDesiredBlockSize() const {return sampleBlockSize;}
	uint getBlockSize() const {return uint(que.vec.size());}

	//take an int16_t, with unbounded max and min value
	void queueSample(const int16_t& samp){
		//std::lock_guard<std::mutex> guard(queWriteControl);
		que.enter();
		que.vec.push_back(samp);
		que.exit();
	}
	//take a floating point value with minimum -1 and maximum 1 value
	void queueSample(const double& samp){
		que.enter();
		que.vec.push_back(int16_t(samp*double(INT16_MAX)));
		que.exit();
	}
	void queueSample(const float& samp){
		que.enter();
		que.vec.push_back(int16_t(samp*float(INT16_MAX)));
		que.exit();
	}

	//get the number of samples wanted in time microseconds
	//regulated to avoid buffer over/underflow
	uint numQueuedIn(uint64_t time){
		return uint(getSampleRate()*(double(time)/1000000.));
	}

};

//get rid of pesky compiler warnings
#define UNUSED(x) (void)(x)

void CALLBACK waveOutCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    UNUSED(dwParam2); 

	AudioStream* pstrm = (AudioStream*)dwInstance;
	if(pstrm->shouldClose) return;
	
	if(uMsg == WOM_CLOSE)
		return;
	
	if(uMsg != WOM_DONE)
		return;
    
    // Retrieve the audio buffer information
    WAVEHDR* pWaveHdr = reinterpret_cast<WAVEHDR*>(dwParam1);
    //int16_t* pBuffer = reinterpret_cast<int16_t*>(pWaveHdr->lpData);
	audioBufferInfo* pinf = (audioBufferInfo*)pWaveHdr->dwUser;

    // Release the audio buffer
    waveOutUnprepareHeader(hwo, pWaveHdr, sizeof(WAVEHDR));

	//swap buffers
	pinf->boundVector->clear();
	pstrm->que.enter();
	std::swap(pstrm->que.vec, *pinf->boundVector);

	//format for new buffer data
	uint writtenSize = uint(pinf->boundVector->size());
	uint writtenSizebuf = writtenSize;

	//append zeroes to extend if too short
	if(writtenSize < 1000){
		if(writtenSize == 0) pinf->boundVector->push_back(0);
		writtenSize = 1000;
		for(uint i = (uint)pinf->boundVector->size(); i<1000; ++i)
			pinf->boundVector->push_back(pinf->boundVector->at(i-1));
	}
	//move into the next stack if too long
	//this is essential to prevent one buffer from becoming very big, and one very small
	else if(writtenSize > pstrm->sampleBlockSize+500){
		for(uint i = pstrm->sampleBlockSize; i<writtenSize; ++i){
			pstrm->que.vec.push_back(pinf->boundVector->at(i));
		}
		writtenSize = pstrm->sampleBlockSize;
	}

	//give the data to waveform audio
	pWaveHdr->lpData = LPSTR(pinf->boundVector->data());
	pWaveHdr->dwUser = DWORD_PTR(pinf);
    pWaveHdr->dwBufferLength = writtenSize * sizeof(int16_t);

	waveOutPrepareHeader(hwo, pWaveHdr, sizeof(WAVEHDR));

	waveOutWrite(hwo, pWaveHdr, sizeof(WAVEHDR));

	//P-regulator for desired sample rate
	double push = pstrm->sampleBlockSize;
	double measure = writtenSizebuf;
	double kp = pstrm->kp;
	double& dest = pstrm->regSampleRate;

	double e = push-measure;
	dest = double(pstrm->sampleRate)+e*kp;
	if(dest < 0) dest = 0;

	pstrm->que.exit();
	
}