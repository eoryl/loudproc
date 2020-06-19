#pragma once
#include "IAudioFilter.h"
class CAmplifier : public IAudioFilter
{
protected:
	float m_fGain;
	float m_fMaxSample;
	int m_iChannelCount;
public:
	//
	CAmplifier();
	~CAmplifier();

	int SetGain(float fGain);
	float GetGain();

	int SetGaindB(float fGaindB);
	float GetGaindB();

	bool HasClipped();

	// Inherited via IAudioFilter
	virtual int Init(int iSampleRate, int iChannels) override;
	virtual int Terminate() override;
	virtual int ProcessFrames(float * pfFrames, int iFrameCount) override;

	// Inherited via IAudioFilter
	virtual int GetDelay() override;
};

