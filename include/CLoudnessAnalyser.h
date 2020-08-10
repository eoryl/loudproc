#pragma once
#include "IAudioFilter.h"
#include "ebur128.h"

class CLoudnessAnalyser : public IAudioFilter
{
	ebur128_state* m_pState;
	double m_dIntegratedLoudness;
	double m_dMomentaryLoudness;
	double m_dShortTermLoudness;
	double m_dLoudnessRange;
	double m_dTruePeak;

public:

	CLoudnessAnalyser();
	virtual ~CLoudnessAnalyser();

	//
	int GetIntegratedLoudness(float* pLUFS);
	int GetMaxMomentaryLoudness(float* pLUFS);
	int GetMaxShortTermLoudness(float* pLUFS);
	int GetLoudnessRange(float* pLU);
	int GetTruePeak(float* pdBFS);

	// Inherited via IAudioFilter
	virtual int Init(int iSampleRate, int iChannels) override;
	virtual int Terminate() override;
	virtual int ProcessFrames(float * pfFrames, int iFrameCount) override;

	// Inherited via IAudioFilter
	virtual int GetDelay() override;
};

