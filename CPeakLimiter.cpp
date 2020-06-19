#include "CPeakLimiter.h"

#include <algorithm>

#include "CAudioTools.h"
#include "errorcodes.h"
#include "macros.h"


int CPeakLimiter::Init(int iSampleRate, int iChannels)
{
	SAFE_DELETE(m_poPeakLimiter);
	m_poPeakLimiter = new PeakLimiter(m_fAttack, m_fRelease, dBFSToLinear(m_fPeakLimitdBFS), iChannels, iSampleRate);
	m_fMaxGainReduction = 0;
	return 0;
}

int CPeakLimiter::Terminate()
{
	SAFE_DELETE(m_poPeakLimiter);
	return 0;
}

int CPeakLimiter::ProcessFrames(float * pfFrames, int iFrameCount)
{
	if (m_poPeakLimiter == NULL) return AP_E_NOT_INITIALISED;

	int procError;

	// FIXME : implement a compensation for the attack delay
	procError = m_poPeakLimiter->applyLimiter_E_I(pfFrames, iFrameCount);

	if (procError != 0) return AP_E_FAIL;

	m_fMaxGainReduction = std::max(m_fMaxGainReduction, fabsf( m_poPeakLimiter->getLimiterMaxGainReduction()));

	return 0;
}

int CPeakLimiter::GetDelay()
{
	if (m_poPeakLimiter != NULL) return m_poPeakLimiter->getLimiterDelay();
    return 0;
}

CPeakLimiter::CPeakLimiter()
{
	m_fAttack = 20.0f;
	m_fRelease = 20.0f;
	m_fPeakLimitdBFS = -3.0f;
	m_poPeakLimiter = NULL;
	m_fMaxGainReduction = 0;
}

CPeakLimiter::~CPeakLimiter()
{
	Terminate();
	SAFE_DELETE(m_poPeakLimiter);
}

int CPeakLimiter::SetAttack(float fAttack_ms)
{
	m_fAttack = fAttack_ms;
	return 0;
}

int CPeakLimiter::SetRelease(float fRelease_ms)
{
	m_fRelease = fRelease_ms;
	return 0;
}

int CPeakLimiter::SetPeakLimit(float fPeakLimitdBFS)
{
	m_fPeakLimitdBFS = fPeakLimitdBFS;
	return 0;
}

float CPeakLimiter::GetAttack()
{
	return m_fAttack;
}

float CPeakLimiter::GetRelease()
{
	return m_fRelease;
}

float CPeakLimiter::GetPeakLimit()
{
	return m_fPeakLimitdBFS;
}
