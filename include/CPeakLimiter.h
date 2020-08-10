#pragma once
#include "IAudioFilter.h"
#include "peakLimiter.h"

class CPeakLimiter : public IAudioFilter
{
	float m_fAttack;
	float m_fRelease;
	float m_fPeakLimitdBFS;

	PeakLimiter* m_poPeakLimiter;

	//stats
	float m_fMaxGainReduction;

public :
	CPeakLimiter();
	virtual ~CPeakLimiter();
	
	int SetAttack(float fAttack_ms);
	int SetRelease(float fRelease_ms);
	int SetPeakLimit(float fPeakLimitdBFS);

	float GetAttack();
	float GetRelease();
	float GetPeakLimit();

	//
	float GetMaxGainReduction();

	// Inherited via IAudioFilter
	virtual int Init(int iSampleRate, int iChannels) override;
	virtual int Terminate() override;
	virtual int ProcessFrames(float * pfFrames, int iFrameCount) override;

	// Inherited via IAudioFilter
	virtual int GetDelay() override;
};

