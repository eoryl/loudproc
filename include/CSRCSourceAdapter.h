#pragma once

#include "IAudioSource.h"
#include "r8bsrc.h"

class CSRCSourceAdapter : public IAudioSource
{
	int m_iSourceSampleRate;
	int m_iTargetSampleRate;
	int m_iChannelCount;
	int m_iBlockDurationms;
	int m_iInBufferSize;
	int m_iInBufferInterleavedSize;


	IAudioSource* m_poSource;

	CR8BResampler * m_poResamplers;
	float* m_pfInBufferInterleaved;
	double** m_ppdInBuffers;
	double** m_ppdOutBuffers;
	int m_iOutBufferSize;
	int m_iOutBufferPos;


public:

	CSRCSourceAdapter();
	virtual ~CSRCSourceAdapter();

	int SetSource(IAudioSource* poAudioSource);

	int SetTargetSampleRate(int iSampleRate);
	int SetBlockDuration(int iDurationMs);

	virtual int Init() override;
	virtual int Terminate() override;
	virtual int GetFormat(int* piSampleRate, int* piChannels) override;
	virtual int GetFrames(float* pfFrames, int iCount, int* iActualCount) override;

};