#include "CAmplifier.h"

#include <algorithm>

#include "errorcodes.h"
#include "macros.h"
#include "CAudioTools.h"

CAmplifier::CAmplifier()
{
	m_fGain = 1.0f;
	m_fMaxSample = 1.0f;
	m_iChannelCount = 0;
}

CAmplifier::~CAmplifier()
{
}

int CAmplifier::SetGain(float fGain)
{
	m_fGain = fGain;
	return 0;
}

float CAmplifier::GetGain()
{
	return m_fGain;
}

int CAmplifier::SetGaindB(float fGain)
{
	m_fGain = dBFSToLinear(m_fGain);
	return 0;
}

float CAmplifier::GetGaindB()
{
	return linearTodBFS(m_fGain);
}

bool CAmplifier::HasClipped()
{
	return (m_fMaxSample > 1.0f);
}

int CAmplifier::Init(int iSampleRate, int iChannels)
{
	m_iChannelCount = iChannels;
	return 0;
}

int CAmplifier::Terminate()
{
	return 0;
}


int CAmplifier::ProcessFrames(float * pfFrames, int iFrameCount)
{
	int i;
	float* pfSample;
	for (i = 0, pfSample = pfFrames; i < iFrameCount * m_iChannelCount; i++, pfSample++)
	{
		*pfSample *= m_fGain;
		m_fMaxSample = std::max(m_fMaxSample, fabsf(*pfSample));
	}
	return 0;
}

int CAmplifier::GetDelay()
{
	return 0;
}
