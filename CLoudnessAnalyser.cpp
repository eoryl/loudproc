#include <math.h>

#include "CLoudnessAnalyser.h"
#include "errorcodes.h"


CLoudnessAnalyser::CLoudnessAnalyser()
{

	m_pState = NULL;
	m_dIntegratedLoudness = -INFINITY;
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


int CLoudnessAnalyser::Init(int iSampleRate, int iChannels)
{
	m_pState= ebur128_init(iChannels, iSampleRate, EBUR128_MODE_I);
	return 0;
}

int CLoudnessAnalyser::Terminate()
{
	if (m_pState != NULL)
	{
		if (m_pState->mode & EBUR128_MODE_I) ebur128_loudness_global(m_pState, &m_dIntegratedLoudness);
		else m_dIntegratedLoudness = -INFINITY;
		//printf("LUFS : %lf\r\n",m_dIntegrageLoudness);
		ebur128_destroy(&m_pState);
		m_pState = NULL;
	}
	return 0;
}

int CLoudnessAnalyser::ProcessFrames(float * pfFrames, int iFrameCount)
{
	if (m_pState == NULL) return AP_E_NOT_INITIALISED;
	if (ebur128_add_frames_float(m_pState, pfFrames, iFrameCount) == 0) return 0;
	else return AP_E_FAIL;
}

int CLoudnessAnalyser::GetDelay()
{
	return 0;
}
