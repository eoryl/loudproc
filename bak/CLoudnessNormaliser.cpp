#include "CLoudnessNormaliser.h"
#include "CAudioTools.h"
#include "CWAVReader.h"
#include "CWAVWriter.h"
#include "CBatchProcess.h"
#include "CAmplifier.h"
#include "CLoudnessAnalyser.h"
#include "CPeakLimiter.h"
#include "CPeakDetector.h"
#include "errorcodes.h"
#include "macros.h"

class CNormaliserInternal
{
public:
	CBatchProcess Processing;
	CWAVReader WaveSource;
	CWAVWriter WaveDestination;
	CAmplifier PreProcAmplifier;
	CPeakLimiter PreProcPeakLimiter;
	CLoudnessAnalyser LoudnessAnalyser;

	CAmplifier Amplifier;
	CPeakLimiter PeakLimiter;
	CPeakDetector PeakDetector;
};

CLoudnessNormaliser::CLoudnessNormaliser()
{
	m_bAnalysed = false;
	//m_fLoudnessRange = 0.0f;
	//m_fMaxMomentaryLoudness = - INFINITY;
	//m_fMaxShorTermLoudness = -INFINITY;
	//m_fIntegratedLoudness = -INFINITY;
	//m_fTruepeak = -INFINITY;

	// normalisation params
	m_fTargetLoudness = -23.0f;
	m_fTargetTruepeak = -1.0f;

	// pre processing params
	m_bPreProcessingAmplifierEnabled = false;
	m_bPreProcessingLimiterEnabled = false;
	//m_fPreProcessingAmplifierGain = 0.0f;
	//m_fPreProcessingLimiterAttack = 20.0f;
	//m_fPreProcessingLimiterRelease = 20.0f;
	//m_fPreProcessingLimiterThreshold = -1.0f;

	// post processing params
	m_bPostProcessingLimiterEnabled = false;
	//m_fPostProcessingLimiterAttack = 20.0f;
	//m_fPostProcessingLimiterRelease = 20.0f;
	//m_fPostProcessingLimiterThreshold = -1.0f;

	m_pfProgressCallback = NULL;
	m_pvProgressCallbackContext = NULL;

	m_poInternal = new CNormaliserInternal();
}

CLoudnessNormaliser::~CLoudnessNormaliser()
{
	SAFE_DELETE(m_poInternal);
}

int CLoudnessNormaliser::SetInputFile(std::wstring oFile)
{
	m_bAnalysed = false;
	m_oInputFIle = oFile;
	return 0;
}

int CLoudnessNormaliser::SetOutputFile(std::wstring oFile)
{
	m_oOutputFIle = oFile;
	return 0;
}

int CLoudnessNormaliser::SetPreProcessingAmplifierEnabled(bool bAmpEnabled)
{
	return 0;
}

int CLoudnessNormaliser::SetPreProcessingAmplifierGain(float fAmpGaindb)
{
	return 0;
}

int CLoudnessNormaliser::SetPreProcessingLimiterEnabled(bool bLimiterEnabled)
{
	return 0;
}

int CLoudnessNormaliser::SetPreProcessingLimiterPeak(float fLimiterPeakdBFS)
{
	return 0;
}

int CLoudnessNormaliser::SetPreProcessingLimiterAttack(float fLimiterAttackms)
{
	return 0;
}

int CLoudnessNormaliser::SetPreProcessingLimiterRelease(float fLimiterReleasems)
{
	return 0;
}

int CLoudnessNormaliser::SetPostProcessingLimiterEnabled(bool bLimiterEnabled)
{
	return 0;
}

int CLoudnessNormaliser::SetPostProcessingLimiterPeak(float fLimiterPeakdBFS)
{
	return 0;
}

int CLoudnessNormaliser::SetPostProcessingLimiterAttack(float fLimiterAttackms)
{
	return 0;
}

int CLoudnessNormaliser::SetPostProcessingLimiterRelease(float fLimiterReleasems)
{
	return 0;
}



int CLoudnessNormaliser::SetTargetLoudness(float fLUFS)
{
	m_fTargetLoudness = fLUFS;
	return 0;
}

int CLoudnessNormaliser::SetTargetLoudnessTruePeak(float fdBFS)
{
	m_fTargetTruepeak = fdBFS;
	return 0;
}



void CLoudnessNormaliser::SetProgressCallback(NORMALISER_CALLBACK pfCallback, void* pvContext)
{
	m_pfProgressCallback = pfCallback;
	m_pvProgressCallbackContext = pvContext;
}

int CLoudnessNormaliser::GetMeasuredTruePeak(float* val)
{
	return m_poInternal->PeakDetector.GetPeakValue(val);
}

int CLoudnessNormaliser::GetMeasuredLoudnessRange(float* val)
{

	//return m_poInternal->LoudnessAnalyser.GetMeasuredLoudnessRange(val);
	return AP_E_NOT_IMPLEMENTED;
}

int CLoudnessNormaliser::GetMeasuredMaxMomentaryLoudness(float* val)
{
	//return m_poInternal->LoudnessAnalyser.GetMeasuredMaxMomentaryLoudness(val);
	return AP_E_NOT_IMPLEMENTED;
}

int CLoudnessNormaliser::GetMeasuredMaxShorTermLoudness(float* val)
{
	//return m_poInternal->LoudnessAnalyser.GetMeasuredMaxShorTermLoudness(val);
	return AP_E_NOT_IMPLEMENTED;
}

int CLoudnessNormaliser::GetMeasuredIntegratedLoudness(float* val)
{
	if (!m_bAnalysed) return AP_E_NOT_INITIALISED;
	return m_poInternal->LoudnessAnalyser.GetIntegratedLoudness(val);
}

int CLoudnessNormaliser::IsLinearNormalisationPossible(bool* pbPossible)
{
	if (!m_bAnalysed) return AP_E_NOT_INITIALISED;
	float fGain = 1.0f, fPeak = 0.0f;
	int iError = GetNormalisationGain(&fGain);
	if (iError) return iError;
	int iError = GetMeasuredTruePeak(&fPeak);
	if (iError) return iError;

	//*pbPossible = ((fGain * m_fTruepeak) <= 1.0f);

	float fTargetTruePeakLinear = dBFSToLinear(m_fTargetTruepeak);
	float fProjectedTruePeak = fGain * fPeak;
	*pbPossible = (fProjectedTruePeak <= fTargetTruePeakLinear);

	return 0;
}

int CLoudnessNormaliser::GetNormalisationGain(float* pfGain)
{
	if (!m_bAnalysed)
		return AP_E_NOT_INITIALISED;

	float fIntegratedLoudness = -INFINITY;
	int iError = GetMeasuredIntegratedLoudness(&fIntegratedLoudness);
	if (iError) return iError;

	float measuredLinear = dBFSToLinear(fIntegratedLoudness * 2.0f);
	float targetLinear = dBFSToLinear(m_fTargetLoudness * 2.0f);
	
	float scaleLinear = targetLinear / measuredLinear;
	//if (mono) scaleLinear /= 2.0;
	// projected 
	*pfGain = sqrtf(scaleLinear);
	return 0;
}

int CLoudnessNormaliser::Analyse()
{

	std::vector<IAudioFilter*> oFilterList;
	int iError;

	m_poInternal->Processing.SetSource(dynamic_cast<IAudioSource*>(&m_poInternal->WaveSource));
	m_poInternal->Processing.SetBlockDuration(40);
	//oProcessing.SetProgressCallback(&CLoudnessNormaliser::ProgessCallback, NULL);
	m_poInternal->Processing.SetProgressCallback(m_pfProgressCallback, m_pvProgressCallbackContext);
	

	if (m_bPreProcessingAmplifierEnabled)
	{
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&m_poInternal->PreProcAmplifier));
	}
	if (m_bPreProcessingLimiterEnabled)
	{
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&m_poInternal->PreProcPeakLimiter));
	}

	oFilterList.push_back(dynamic_cast<IAudioFilter*>(&m_poInternal->LoudnessAnalyser));
	oFilterList.push_back(dynamic_cast<IAudioFilter*>(&m_poInternal->PeakDetector));
	m_oProcessing.SetFilters(oFilterList);
	m_oWaveSource.SetFileName(m_oInputFIle);
	iError = m_oProcessing.ProcessAllFrames();
	if (iError)	return iError;
	iError = m_oLoudnessAnalyser.GetIntegratedLoudness(&m_fIntegratedLoudness);
	if (iError)	return iError;
	iError = m_oPeakDetector.GetPeakValue(&m_fTruepeak);
	if (iError)	return iError;
	m_bAnalysed = true;
	return 0;
}

int CLoudnessNormaliser::Normalise()
{
	int iError = 0;
	if (!m_bAnalysed) return AP_E_INVALID_STATE;
	

	bool bLinearPossible = false;
	iError = IsLinearNormalisationPossible(&bLinearPossible);
	if (iError) return iError;

	if (!bLinearPossible  && !m_bPostProcessingLimiterEnabled) return AP_E_OPERATION_NOT_POSSIBLE;




	std::vector<IAudioFilter*> oFilterList;

	m_oProcessing.SetSource(dynamic_cast<IAudioSource*>(&m_oWaveSource));
	m_oProcessing.SetBlockDuration(40);
	//oProcessing.SetProgressCallback(&CLoudnessNormaliser::ProgessCallback, NULL);
	m_oProcessing.SetProgressCallback(m_pfProgressCallback, m_pvProgressCallbackContext);


	if (m_bPreProcessingAmplifierEnabled)
	{
		m_oPreProcAmplifier.SetGaindB(m_fPreProcessingAmplifierGain);
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&m_oPreProcAmplifier));
	}
	if (m_bPreProcessingLimiterEnabled)
	{
		m_oPreProcPeakLimiter.SetAttack(m_fPreProcessingLimiterAttack);
		m_oPreProcPeakLimiter.SetRelease(m_fPreProcessingLimiterRelease);
		m_oPreProcPeakLimiter.SetPeakLimit(m_fPreProcessingLimiterThreshold);
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&m_oPreProcPeakLimiter));
	}

	//
	float fGain = 1.0f;
	iError = GetNormalisationGain(&fGain);
	if (iError) return iError;
	if (fGain != 1.0f)
	{
		m_oAmplifier.SetGain(fGain);
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&m_oAmplifier));
		if (m_bPostProcessingLimiterEnabled)
		{
			m_oPeakLimiter.SetAttack(m_fPostProcessingLimiterAttack);
			m_oPeakLimiter.SetRelease(m_fPostProcessingLimiterRelease);
			m_oPeakLimiter.SetPeakLimit(m_fPostProcessingLimiterThreshold);
			oFilterList.push_back(dynamic_cast<IAudioFilter*>(&m_oPeakLimiter));
		}
	}
	oFilterList.push_back(dynamic_cast<IAudioFilter*>(&m_oLoudnessAnalyser));
	oFilterList.push_back(dynamic_cast<IAudioFilter*>(&m_oPeakDetector));
	m_oProcessing.SetFilters(oFilterList);

	m_oWaveSource.SetFileName(m_oInputFIle);
	m_oWaveDestination.SetFileName(m_oOutputFIle);
	iError = m_oProcessing.ProcessAllFrames();
	if (iError)	return iError;
	iError = m_oLoudnessAnalyser.GetIntegratedLoudness(&m_fIntegratedLoudness);
	if (iError)	return iError;
	iError = m_oPeakDetector.GetPeakValue(&m_fTruepeak);
	if (iError)	return iError;
	m_bAnalysed = true;

	return 0;
}

int CLoudnessNormaliser::ProgessCallback(int status, unsigned long long processed, void* ctx)
{
	switch ((BatchProcessProgressStatus)status)
	{
	case BPS_END :
		printf("\r\n", processed);
		break;
	case BPS_ERROR:
		printf("\r\nError\r\n", processed);
		break;
	case BPS_PROCESS:
		printf("frames processed: %ld \r", processed);
		break;
	}
	return 0;
}
