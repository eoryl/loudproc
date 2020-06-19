#pragma once

/*
Wrapper over Windows MMIO API (over yet another wrapper) to read from WAV files.
Note that the MMIO API seems to be limited to vanilla WAV files that have limit 
of 2GB. A bit over 3 hours long for PCM 16 bit 48000 stereo files.
*/

#include <string>
#include "IAudioSource.h"
#include "wave.h"

class CWAVReader : public IAudioSource
{
	std::wstring m_oFile;

	// mmio api
	HMMIO m_hmmio;
	MMIOINFO m_mmioInf;
	WAVEFORMATEX* m_pWfx;
	MMCKINFO m_mmcki, m_mmckiRIFF;

	// internal copy of format
	WAVEFORMATEX m_oFormat;

	// internal read buffer
	BYTE* m_pucBuffer;
	int m_iBufferSize;

	// stats
	unsigned long m_ulTotalBytesRead;

	//
	int SetInternalBufferSize(int iBytes);


public:

	CWAVReader();
	~CWAVReader();

	int SetFileName(std::wstring oFile);
	std::wstring GetFileName();

	int OpenFile();
	int CloseFile();

	int GetFileFormat(WAVEFORMATEX * pWFX);
	int GetFileDuration(unsigned long * pulDuration_ms);;

	// Inherited via IAudioSource
	virtual int Init() override;
	virtual int Terminate() override;
	virtual int GetFormat(int* piSampleRate, int* piChannels) override;
	virtual int GetFrames(float* pfFrames, int iCount, int* iActualCount) override;
};

