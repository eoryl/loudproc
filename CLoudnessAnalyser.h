#pragma once
#include "IAudioFilter.h"
#include "ebur128.h"

class CLoudnessAnalyser : public IAudioFilter
{
	ebur128_state* m_pState;
	double m_dIntegratedLoudness;


public:

	CLoudnessAnalyser();
	~CLoudnessAnalyser();

	//
	int GetIntegratedLoudness(float* pLUFS);

	// Inherited via IAudioFilter
	virtual int Init(int iSampleRate, int iChannels) override;
	virtual int Terminate() override;
	virtual int ProcessFrames(float * pfFrames, int iFrameCount) override;

	// Inherited via IAudioFilter
	virtual int GetDelay() override;
};

