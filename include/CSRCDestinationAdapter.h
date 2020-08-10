#pragma once

#include "IAudioFilter.h"
#include "r8bsrc.h"

class CSRCDestinationAdapter : public IAudioFilter
{
	int m_iSourceSampleRate;
	int m_iTargetSampleRate;
	int m_iChannelCount;
	int m_iBlockDurationms;
	bool m_b24bit;


	IAudioDestination * m_poDestination;

	CR8BResampler* m_poResamplers;
	double** m_ppdInBuffers;

	double** m_ppdOutBuffers;
	int m_iOutBufferSize;

	float* m_pfOutBufferInterleaved;
	int m_iOutBufferInterleavedSize;


public:

	CSRCDestinationAdapter();
	virtual ~CSRCDestinationAdapter();

	virtual int SetDestination(IAudioDestination* poDestination);
	int SetTargetSampleRate(int iSampleRate);
	int SetBlockDuration(int iDurationMs);
	int Set24bitOutput(bool b24);

	// Inherited via IAudioDestination
	virtual int Init(int iSampleRate, int iChannels) override;

	virtual int Terminate() override;

	virtual int ProcessFrames(float* pfFrames, int iSampleCount) override;

	virtual int GetDelay() override;

};
