#pragma once
#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>

#include <Windows.h>

#ifndef FTYPE
#define FTYPE double
#endif

#ifndef PI
#define PI 2.0 * std::acos(0.0)
#endif

template<class T>
class olcNoiseMaker{
	private: FTYPE(*m_userFunction)(int, FTYPE) = NULL;
	private: size_t m_nSampleRate = 0;
	private: size_t m_nChannels = 0;
	private: size_t m_nBlockCount = 0;
	private: size_t m_nBlockSamples = 0;
	private: size_t m_nBlockCurrent = 0;

	private: T* m_pBlockMemory = nullptr;
	private: WAVEHDR* m_pWaveHeaders = nullptr;
	private: HWAVEOUT m_hwDevice = NULL;

	private: std::thread m_thread;
	private: std::atomic<bool> m_bReady = false;
	private: std::atomic<size_t> m_nBlockFree = 0;
	private: std::atomic<FTYPE> m_dGlobalTime = 0.0;
	private: std::condition_variable m_cvBlockNotZero;
	private: std::mutex m_muxBlockNotZero;

	// Handler for soundcard request for more data
	private: void waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2) {
		if (uMsg != WOM_DONE)
			return;
		m_nBlockFree++;
		std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
		m_cvBlockNotZero.notify_one();
	}
	// Static wrapper for sound card handler
	private: static void CALLBACK waveOutProcWrap(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
		((olcNoiseMaker*)dwInstance)->waveOutProc(hWaveOut, uMsg, dwParam1, dwParam2);
	}
	// Main thread. This loop responds to requests from the soundcard to fill 'blocks'
	// with audio data. If no requests are available it goes dormant until the sound
	// card is ready for more data. The block is fille by the "user" in some manner
	// and then issued to the soundcard.
	private: void MainThread() {
		m_dGlobalTime = 0.0;
		FTYPE dTimeStep = 1.0 / (FTYPE)m_nSampleRate;

		// Goofy hack to get maximum integer for a type at run-time
		T nMaxSample = (T)pow(2, (sizeof(T) * 8) - 1) - 1;
		FTYPE dMaxSample = (FTYPE)nMaxSample;
		T nPreviousSample = 0;

		while (m_bReady) {
			// Wait for block to become available
			if (m_nBlockFree == 0) {
				std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
				m_cvBlockNotZero.wait(lm);
			}

			// Block is here, so use it
			m_nBlockFree--;

			// Prepare block for processing
			if (m_pWaveHeaders[m_nBlockCurrent].dwFlags & WHDR_PREPARED)
				waveOutUnprepareHeader(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));

			T nNewSample = 0;
			int nCurrentBlock = m_nBlockCurrent * m_nBlockSamples;

			for (size_t n = 0; n < m_nBlockSamples; n += m_nChannels) {
				for (size_t c = 0; c < m_nChannels; c++) {
					// User Process
					if (m_userFunction == nullptr)
						nNewSample = (T)(clip(UserProcess(c, m_dGlobalTime), 1.0) * dMaxSample);
					else
						nNewSample = (T)(clip(m_userFunction(c, m_dGlobalTime), 1.0) * dMaxSample);

					m_pBlockMemory[nCurrentBlock + n + c] = nNewSample;
					nPreviousSample = nNewSample;
				}
				m_dGlobalTime = m_dGlobalTime + dTimeStep;
			}

			// Send block to sound device
			waveOutPrepareHeader(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));
			waveOutWrite(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));
			m_nBlockCurrent++;
			m_nBlockCurrent %= m_nBlockCount;
		}
	}

	public: olcNoiseMaker(){}
	public: olcNoiseMaker(std::wstring sOutputDevice, size_t nSampleRate = 44100, size_t nChannels = 1, size_t nBlocks = 8, size_t nBlockSamples = 512) {
		Create(sOutputDevice, nSampleRate, nChannels, nBlocks, nBlockSamples);
	}
	
	public: bool Create(std::wstring sOutputDevice, size_t nSampleRate = 44100, size_t nChannels = 1, size_t nBlocks = 8, size_t nBlockSamples = 512){
		this->m_bReady = false;
		this->m_nSampleRate = nSampleRate;
		this->m_nChannels = nChannels;
		this->m_nBlockCount = nBlocks;
		this->m_nBlockSamples = nBlockSamples;
		this->m_nBlockFree = m_nBlockCount;
		this->m_nBlockCurrent = 0;
		this->m_pBlockMemory = nullptr;
		this->m_pWaveHeaders = nullptr;

		m_userFunction = nullptr;

		// Validate device
		std::vector<std::wstring> devices = Enumerate();
		auto d = std::find(devices.begin(), devices.end(), sOutputDevice);
		if (d != devices.end()){
			// Device is available
			int nDeviceID = distance(devices.begin(), d);
			WAVEFORMATEX waveFormat;
			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			waveFormat.nSamplesPerSec = m_nSampleRate;
			waveFormat.wBitsPerSample = sizeof(T) * 8;
			waveFormat.nChannels = m_nChannels;
			waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
			waveFormat.cbSize = 0;

			// Open Device if valid
			if (waveOutOpen(&m_hwDevice, nDeviceID, &waveFormat, (DWORD_PTR)waveOutProcWrap, (DWORD_PTR)this, CALLBACK_FUNCTION) != S_OK)
				return false;
		}

		// Allocate Wave|Block Memory
		m_pBlockMemory = new T[m_nBlockCount * m_nBlockSamples];
		if (m_pBlockMemory == nullptr)
			return false;
		ZeroMemory(m_pBlockMemory, sizeof(T) * m_nBlockCount * m_nBlockSamples);

		m_pWaveHeaders = new WAVEHDR[m_nBlockCount];
		if (m_pWaveHeaders == nullptr)
			return false;
		ZeroMemory(m_pWaveHeaders, sizeof(WAVEHDR) * m_nBlockCount);

		// Link headers to block memory
		for (size_t n = 0; n < m_nBlockCount; n++){
			m_pWaveHeaders[n].dwBufferLength = m_nBlockSamples * sizeof(T);
			m_pWaveHeaders[n].lpData = (LPSTR)(m_pBlockMemory + (n * m_nBlockSamples));
		}

		m_bReady = true;

		m_thread = std::thread(&olcNoiseMaker::MainThread, this);

		// Start the ball rolling
		std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
		m_cvBlockNotZero.notify_one();

		return true;
	}
	public: void Stop(){
		m_bReady = false;
		m_thread.join();
	}
	// Override to process current sample
	public: virtual FTYPE UserProcess(int nChannel, FTYPE dTime){
		return 0.0;
	}
	public: FTYPE GetTime(){
		return m_dGlobalTime;
	}
	public: static std::vector<std::wstring> Enumerate(){
		int nDeviceCount = waveOutGetNumDevs();
		std::vector<std::wstring> sDevices;
		WAVEOUTCAPS woc;
		for (int n = 0; n < nDeviceCount; n++)
			if (waveOutGetDevCaps(n, &woc, sizeof(WAVEOUTCAPS)) == S_OK)
				sDevices.push_back(woc.szPname);
		return sDevices;
	}
	public: void SetUserFunction(FTYPE(*func)(int, FTYPE)){
		m_userFunction = func;
	}
	public: FTYPE clip(FTYPE dSample, FTYPE dMax){
		if (dSample >= 0.0)
			return fmin(dSample, dMax);
		else
			return fmax(dSample, -dMax);
	}
};