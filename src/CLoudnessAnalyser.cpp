#include <math.h>

#include "CLoudnessAnalyser.h"
#include "errorcodes.h"


CLoudnessAnalyser::CLoudnessAnalyser()
{

	m_pState = NULL;
	m_dIntegratedLoudness = -INFINITY;
	m_dMaxMomentaryLoudness = -INFINITY;
	m_dMaxShortTermLoudness = -INFINITY;
	m_dMomentaryLoudness = -INFINITY;
	m_dShortTermLoudness = -INFINITY;
	m_dTruePeak = -INFINITY;
	m_dLoudnessRange = 0;

}

CLoudnessAnalyser::~CLoudnessAnalyser() 
{
	Terminate();
}

int CLoudnessAnalyser::GetIntegratedLoudness(float* pLUFS)
{
	*pLUFS = m_dIntegratedLoudness;
	return 0;
}

int CLoudnessAnalyser::GetMaxMomentaryLoudness(float* pLUFS)
{
	*pLUFS = m_dMaxMomentaryLoudness;
	return 0;
}

int CLoudnessAnalyser::GetMaxShortTermLoudness(float* pLUFS)
{
	*pLUFS = m_dMaxShortTermLoudness;
	return 0;
}

int CLoudnessAnalyser::GetLoudnessRange(float* pLU)
{
	*pLU = m_dLoudnessRange;
	return 0;
}

int CLoudnessAnalyser::GetTruePeak(float* pdBFS)
{
	*pdBFS = m_dTruePeak;
	return 0;
}


int CLoudnessAnalyser::Init(int iSampleRate, int iChannels)
{
	m_pState= ebur128_init(iChannels, iSampleRate, EBUR128_MODE_I | EBUR128_MODE_TRUE_PEAK | EBUR128_MODE_LRA);
	m_dIntegratedLoudness = -INFINITY;
	m_dMaxMomentaryLoudness = -INFINITY;
	m_dMaxShortTermLoudness = -INFINITY;
	m_dMomentaryLoudness = -INFINITY;
	m_dShortTermLoudness = -INFINITY;
	m_dTruePeak = 0;
	m_dLoudnessRange = 0;
	
	return 0;
}

int CLoudnessAnalyser::Terminate()
{
	if (m_pState != NULL)
	{
		UpdateValues();

		//printf("LUFS : %lf\r\n",m_dIntegrageLoudness);
		ebur128_destroy(&m_pState);
		m_pState = NULL;
	}
	return 0;
}

int CLoudnessAnalyser::ProcessFrames(float * pfFrames, int iFrameCount)
{
	// TODO: Consider breaking down in sub 400ms chuncks 
	// if method receives over 400ms for accuracy of shortterm loudness measurement
	if (m_pState == NULL) return AP_E_NOT_INITIALISED;
	if (ebur128_add_frames_float(m_pState, pfFrames, iFrameCount) != 0) return  AP_E_FAIL;

	UpdateValues();

	return 0;
}

void CLoudnessAnalyser::UpdateValues()
{
	if (m_pState->mode & EBUR128_MODE_I)
	{
		ebur128_loudness_global(m_pState, &m_dIntegratedLoudness);
	}

	if (m_pState->mode & EBUR128_MODE_M)
	{
		ebur128_loudness_momentary(m_pState, &m_dMomentaryLoudness);
		if (m_dMomentaryLoudness > m_dMaxMomentaryLoudness) m_dMaxMomentaryLoudness = m_dMomentaryLoudness;
	}

	if (m_pState->mode & EBUR128_MODE_S)
	{
		ebur128_loudness_shortterm(m_pState, &m_dShortTermLoudness);
		if (m_dShortTermLoudness > m_dMaxShortTermLoudness) m_dMaxShortTermLoudness = m_dShortTermLoudness;
	}

	if (m_pState->mode & EBUR128_MODE_LRA)
	{
		ebur128_loudness_range(m_pState, &m_dLoudnessRange);
	}
	if (m_pState->mode & EBUR128_MODE_TRUE_PEAK)
	{
		double dChTP = -INFINITY;
		for (int c = 0; c < m_pState->channels; c++)
		{
			ebur128_true_peak(m_pState, c, &dChTP);
			if (dChTP > m_dTruePeak) m_dTruePeak = dChTP;
		}
	}
}

int CLoudnessAnalyser::GetDelay()
{
	return 0;
}
