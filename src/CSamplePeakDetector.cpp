#include "CSamplePeakDetector.h"

#include <algorithm>

#include "macros.h"
#include "errorcodes.h"
#include "CAudioTools.h"

CSamplePeakDetector::CSamplePeakDetector()
{
	m_iChannelCount = 0;
	m_pfPeaks = NULL;
}

CSamplePeakDetector::~CSamplePeakDetector()
{
	SAFE_ARRAY_DELETE(m_pfPeaks);
}

int CSamplePeakDetector::GetPeakValue(float* peak)
{
	if (m_pfPeaks == NULL) return AP_E_NOT_INITIALISED;
	*peak = 0;
	for (int i = 0; i < m_iChannelCount; i++) *peak = std::max(*peak, m_pfPeaks[i]);

	return 0;
}

int CSamplePeakDetector::GetPeakValue(float* peak, int channel)
{
	if (m_pfPeaks == NULL) return AP_E_NOT_INITIALISED;
	if ((channel < 0) || (channel >= m_iChannelCount)) return AP_E_INVALID_ARGUMENT;
	*peak = m_pfPeaks[channel];
	return 0;
}

bool CSamplePeakDetector::HasClipped()
{
	float fPeak;
	if (GetPeakValue(&fPeak) != 0) return false;
	return (fPeak > 1.0f);
}

int CSamplePeakDetector::Init(int iSampleRate, int iChannels)
{
	m_iChannelCount = 0;
	SAFE_ARRAY_DELETE(m_pfPeaks);
	if (iChannels > 0)
	{
		m_pfPeaks = new float[iChannels];
		m_iChannelCount = iChannels;
	}

	return 0;
}

int CSamplePeakDetector::Terminate()
{
	return 0;
}

int CSamplePeakDetector::ProcessFrames(float * pfFrames, int iFrameCount)
{
	if (m_pfPeaks == NULL) return AP_E_NOT_INITIALISED;
	int i;
	float* pfSample;
	for (i = 0, pfSample = pfFrames; i < iFrameCount * m_iChannelCount; i++, pfSample++)
	{
		float * pfChPeak = m_pfPeaks + (i % m_iChannelCount) ;
		*pfChPeak = std::max(*pfChPeak, fabsf(*pfSample));
	}
	return 0;
}

int CSamplePeakDetector::GetDelay()
{
	return 0;
}
