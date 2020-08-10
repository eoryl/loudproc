#pragma once

/*
Wrapper over Windows MMIO API (over yet another wrapper) to write WAV files.
Note that the MMIO API seems to be limited to vanilla WAV files that have limit
of 2GB. A bit over 3 hours long for PCM 16 bit 48000 stereo files.
*/

#include <string>
#include "IAudioFilter.h"
#include "wave.h"

class CWAVWriter : public IAudioFilter
{
	// mmio API
    HMMIO m_hmmio ;
    MMIOINFO m_mmioInf;
    MMCKINFO m_mmcki, m_mmckiRIFF;

	// file and format
	std::wstring m_oFileName;
	WAVEFORMATEX m_oWfx;

	// internal read buffer
	BYTE* m_pucBuffer;
	int m_iBufferSize;

	// stats
	int m_iTotalBytesWritten;

public:

	CWAVWriter();
	virtual ~CWAVWriter();


	int SetFileName(std::wstring oFile);
	std::wstring GetFileName();
	// int CloseFile();
	// int SetTargetFormat(int iSampleRate, int iChannels, int iBitDepth);
	// int GetTargetFormat(int * piSampleRate, int* piChannels, int* piBitDepth);

	int SetInternalBufferSize(int iBytes);

	// Inherited via IAudioFilter
	virtual int Init(int iSampleRate, int iChannels) override;
	virtual int Terminate() override;
	virtual int ProcessFrames(float * pfFrames, int iFrameCount) override;

	// Inherited via IAudioFilter
	virtual int GetDelay() override;
};

