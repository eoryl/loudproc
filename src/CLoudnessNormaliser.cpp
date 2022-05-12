// for time counter
#include <Windows.h>
#include <timeapi.h>

//
#include "CLoudnessNormaliser.h"
#include "CAudioTools.h"
#include "CWAVReader.h"
#include "CWAVWriter.h"
#include "CBatchProcess.h"
#include "CAmplifier.h"
#include "CLoudnessAnalyser.h"
#include "CPeakLimiter.h"
#include "CSamplePeakDetector.h"
#include "CSRCSourceAdapter.h"
#include "CSRCDestinationAdapter.h"
#include "errorcodes.h"
#include "CStringTools.h"


CLoudnessNormaliser::CLoudnessNormaliser()
{
	m_bAnalysed = false;
	m_fLoudnessRange = 0.0f;
	m_fMaxMomentaryLoudness = -INFINITY;
	m_fMaxShortTermLoudness = -INFINITY;
	m_fIntegratedLoudness = -INFINITY;
	m_fTruepeak = -INFINITY;
	m_fSamplepeak = -INFINITY;
	m_iSampleRate = 48000;
	m_iChannel = 2;
	m_bMonoAttenuation = false;

	// normalisation params
	m_fTargetLoudness = -23.0f;
	m_fTargetTruepeak = -1.0f;

	// pre processing params
	m_bPreProcessingAmplifierEnabled = false;
	m_fPreProcessingAmplifierGain = 0.0f;
	m_bPreProcessingLimiterEnabled = false;
	m_fPreProcessingLimiterAttack = 20.0f;
	m_fPreProcessingLimiterRelease = 20.0f;
	m_fPreProcessingLimiterThreshold = -1.0f;
	m_fPreProcessingLimiterMaxGainReduction = 0.0f;

	// post processing params
	m_bPostProcessingLimiterEnabled = false;
	m_fPostProcessingLimiterAttack = 20.0f;
	m_fPostProcessingLimiterRelease = 20.0f;
	m_fPostProcessingLimiterThreshold = -1.0f;
	m_fPostProcessingLimiterMaxGainReduction = 0.0f;
	m_pfProgressCallback = NULL;
	m_pvProgressCallbackContext = NULL;

	// upsampling
	m_bEnableUpsampling = false;

	// stats
	m_llAnalysisTime = 0;
	m_llNormalisationTime = 0;


}

CLoudnessNormaliser::~CLoudnessNormaliser()
{
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

int CLoudnessNormaliser::SetPreProcessing(bool bAmpEnabled, float fAmpGaindb, bool bLimiterEnabled,
	float fLimiterPeakdBFS, float fLimiterAttackms, float fLimiterReleasems)
{
	m_bAnalysed = false;
	m_bPreProcessingAmplifierEnabled = bAmpEnabled;
	m_fPreProcessingAmplifierGain = fAmpGaindb;
	m_bPreProcessingLimiterEnabled = bLimiterEnabled;
	m_fPreProcessingLimiterThreshold = fLimiterPeakdBFS;
	m_fPreProcessingLimiterAttack = fLimiterAttackms;
	m_fPreProcessingLimiterRelease = fLimiterReleasems;

	return 0;
}

int CLoudnessNormaliser::SetPostProcessing(bool bLimiterEnabled, float fLimiterPeakdBFS, float fLimiterAttackms,
	float fLimiterReleasems)
{
	m_bPostProcessingLimiterEnabled = bLimiterEnabled;
	m_fPostProcessingLimiterThreshold = fLimiterPeakdBFS;
	m_fPostProcessingLimiterAttack = fLimiterAttackms;
	m_fPostProcessingLimiterRelease = fLimiterReleasems;
	return 0;
}

int CLoudnessNormaliser::SetUpsamplingEnabled(bool bEnabled)
{
	m_bEnableUpsampling = bEnabled;
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

int CLoudnessNormaliser::SetMonoAttenution(bool bEnabled)
{
	m_bMonoAttenuation = bEnabled;
	return 0;
}


void CLoudnessNormaliser::SetProgressCallback(NORMALISER_CALLBACK pfCallback, void* pvContext)
{
	m_pfProgressCallback = pfCallback;
	m_pvProgressCallbackContext = pvContext;
}

int CLoudnessNormaliser::GetMeasuredTruePeak(float* val)
{
	*val = m_fTruepeak;
	return 0;
}

int CLoudnessNormaliser::GetMeasuredSamplePeak(float* val)
{
	*val = m_fSamplepeak;
	return 0;
}

int CLoudnessNormaliser::GetMeasuredLoudnessRange(float* val)
{
	if (!m_bAnalysed) return AP_E_NOT_INITIALISED;
	*val = m_fLoudnessRange;
	return 0;
}

int CLoudnessNormaliser::GetMeasuredMaxMomentaryLoudness(float* val)
{
	if (!m_bAnalysed) return AP_E_NOT_INITIALISED;
	*val = m_fMaxMomentaryLoudness;
	return 0;
}

int CLoudnessNormaliser::GetMeasuredMaxShorTermLoudness(float* val)
{
	if (!m_bAnalysed) return AP_E_NOT_INITIALISED;
	*val = m_fMaxShortTermLoudness;
	return 0;
}

int CLoudnessNormaliser::GetMeasuredIntegratedLoudness(float* val)
{
	if (!m_bAnalysed) return AP_E_NOT_INITIALISED;
	*val = m_fIntegratedLoudness;
	return 0;
}

int CLoudnessNormaliser::GetPreProcessingStats(float* pfLimiterMaxGainReductiondB)
{
	*pfLimiterMaxGainReductiondB = m_fPreProcessingLimiterMaxGainReduction;
	return 0;
}

int CLoudnessNormaliser::GetPostProcessingStats(float* pfLimiterMaxGainReductiondB)
{
	*pfLimiterMaxGainReductiondB = m_fPostProcessingLimiterMaxGainReduction;
	return 0;
}

int CLoudnessNormaliser::GetAnalysisTime(long long* pllAnalysisTime)
{
	*pllAnalysisTime = m_llAnalysisTime;
	return 0;
}

int CLoudnessNormaliser::GetNormalisationTime(long long* pllNormalisationTime)
{
	*pllNormalisationTime = m_llNormalisationTime;
	return 0;
}

int CLoudnessNormaliser::GetFileDuration(long long* pllFileDuration)
{
	CWAVReader oSrcFile;
	int iError;
	unsigned long ulDuration = 0;
	
	iError = oSrcFile.SetFileName(m_oInputFIle);
	if (iError) return iError;
	
	iError = oSrcFile.GetFileDuration(&ulDuration);
	if (iError) return iError;

	*pllFileDuration = ulDuration;

	return 0;
}


int CLoudnessNormaliser::IsLinearNormalisationPossible(bool* pbPossible)
{
	if (!m_bAnalysed) return AP_E_NOT_INITIALISED;
	float fGain = 1.0f;
	int iError = GetNormalisationGain(&fGain);
	if (iError) return iError;
	//*pbPossible = ((fGain * m_fTruepeak) <= 1.0f);

	float fTargetTruePeakLinear = dBFSToLinear(m_fTargetTruepeak);
	float fProjectedTruePeak = fGain * m_fTruepeak;
	*pbPossible = (fProjectedTruePeak <= fTargetTruePeakLinear);

	return 0;
}

int CLoudnessNormaliser::GetNormalisationGain(float* pfGain)
{
	if (!m_bAnalysed)
		return AP_E_NOT_INITIALISED;

	float fTargetLoudness = m_fTargetLoudness;
	//mono management method 1 - remove 3 LUFS 
	//if ((m_iChannel == 1 ) && (m_bMonoAttenuation)) fTargetLoudness -=3.0f;

	float measuredLinear = dBFSToLinear(m_fIntegratedLoudness * 2.0f);
	float targetLinear = dBFSToLinear(fTargetLoudness * 2.0f);

	float scaleLinear = targetLinear / measuredLinear;
	
	//mono management method 2 - divide scale factor by 2
	if ((m_iChannel == 1 ) && (m_bMonoAttenuation)) scaleLinear /= 2.0;

	// projected 
	*pfGain = sqrtf(scaleLinear);
	return 0;
}

int CLoudnessNormaliser::GetConstrainedNormalisationGain(float* pfGain)
{
	if (!m_bAnalysed)
		return AP_E_NOT_INITIALISED;


	return 0;
}

int CLoudnessNormaliser::Analyse()
{
	CBatchProcess oProcessing;
	CWAVReader oWaveSource;
	CWAVWriter oWaveDestination;
	CAmplifier oPreProcAmplifier;
	CPeakLimiter oPreProcPeakLimiter;
	CLoudnessAnalyser oLoudnessAnalyser;
	CSamplePeakDetector oPeakDetector;
	CSRCSourceAdapter oSourceSampleRateConverter;

	std::vector<IAudioFilter*> oFilterList;
	int iError;

	// stats
	DWORD dwStartTime = timeGetTime();
	m_llAnalysisTime = 0;

	if (m_bEnableUpsampling)
	{
		oSourceSampleRateConverter.SetBlockDuration(400);
		oSourceSampleRateConverter.SetTargetSampleRate(192000);
		oSourceSampleRateConverter.SetSource( dynamic_cast<IAudioSource*>(&oWaveSource));
		oProcessing.SetSource(dynamic_cast<IAudioSource*>(&oSourceSampleRateConverter));
	}
	else
	{
		oProcessing.SetSource(dynamic_cast<IAudioSource*>(&oWaveSource));
	}
	// we will use 400ms blocks to match momentary loudness window
	oProcessing.SetBlockDuration(400);
	//oProcessing.SetProgressCallback(&CLoudnessNormaliser::ProgessCallback, NULL);
	oProcessing.SetProgressCallback(m_pfProgressCallback, m_pvProgressCallbackContext);


	if (m_bPreProcessingAmplifierEnabled)
	{
		oPreProcAmplifier.SetGaindB(m_fPreProcessingAmplifierGain);
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oPreProcAmplifier));
	}
	if (m_bPreProcessingLimiterEnabled)
	{
		oPreProcPeakLimiter.SetAttack(m_fPreProcessingLimiterAttack);
		oPreProcPeakLimiter.SetRelease(m_fPreProcessingLimiterRelease);
		oPreProcPeakLimiter.SetPeakLimit(m_fPreProcessingLimiterThreshold);
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oPreProcPeakLimiter));
	}

	oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oLoudnessAnalyser));
	oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oPeakDetector));
	oProcessing.SetFilters(oFilterList);
	oWaveSource.SetFileName(m_oInputFIle);

	iError = oWaveSource.GetFormat(&m_iSampleRate, &m_iChannel);
	if (iError)	return iError;

	iError = oProcessing.ProcessAllFrames();
	if (iError)	return iError;

	DWORD dwEndTime = timeGetTime();
	m_llAnalysisTime = dwEndTime - dwStartTime;

	iError = oLoudnessAnalyser.GetIntegratedLoudness(&m_fIntegratedLoudness);
	if (iError)	return iError;
	iError = oLoudnessAnalyser.GetTruePeak(&m_fTruepeak);
	if (iError)	return iError;
	iError = oPeakDetector.GetPeakValue(&m_fSamplepeak);
	if (iError)	return iError;
	iError = oLoudnessAnalyser.GetMaxMomentaryLoudness(&m_fMaxMomentaryLoudness );
	if (iError)	return iError;
	iError = oLoudnessAnalyser.GetMaxShortTermLoudness(&m_fMaxShortTermLoudness);
	if (iError)	return iError;
	iError = oLoudnessAnalyser.GetLoudnessRange(&m_fLoudnessRange);
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

	if (!bLinearPossible && !m_bPostProcessingLimiterEnabled) return AP_E_OPERATION_NOT_POSSIBLE;

	// stats
	m_llNormalisationTime = 0;
	DWORD dwStartTime = timeGetTime();

	CBatchProcess oProcessing;
	CWAVReader oWaveSource;
	CWAVWriter oWaveDestination;
	CAmplifier oPreProcAmplifier;
	CPeakLimiter oPreProcPeakLimiter;
	CLoudnessAnalyser oLoudnessAnalyser;

	CAmplifier oAmplifier;
	CPeakLimiter oPeakLimiter;
	CSamplePeakDetector oPeakDetector;

	std::vector<IAudioFilter*> oFilterList;

	CSRCSourceAdapter oSourceSampleRateConverter;
	CSRCDestinationAdapter oDestinationSampleRateConverter;


	if (m_bEnableUpsampling)
	{
		oSourceSampleRateConverter.SetBlockDuration(400);
		oSourceSampleRateConverter.SetTargetSampleRate(192000);
		oSourceSampleRateConverter.SetSource(dynamic_cast<IAudioSource*>(&oWaveSource));
		oProcessing.SetSource(dynamic_cast<IAudioSource*>(&oSourceSampleRateConverter));
	}
	else
	{
		oProcessing.SetSource(dynamic_cast<IAudioSource*>(&oWaveSource));
	}

	oProcessing.SetBlockDuration(400);
	//oProcessing.SetProgressCallback(&CLoudnessNormaliser::ProgessCallback, NULL);
	oProcessing.SetProgressCallback(m_pfProgressCallback, m_pvProgressCallbackContext);


	if (m_bPreProcessingAmplifierEnabled)
	{
		oPreProcAmplifier.SetGaindB(m_fPreProcessingAmplifierGain);
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oPreProcAmplifier));
	}
	if (m_bPreProcessingLimiterEnabled)
	{
		oPreProcPeakLimiter.SetAttack(m_fPreProcessingLimiterAttack);
		oPreProcPeakLimiter.SetRelease(m_fPreProcessingLimiterRelease);
		oPreProcPeakLimiter.SetPeakLimit(m_fPreProcessingLimiterThreshold);
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oPreProcPeakLimiter));
	}

	//
	float fGain = 1.0f;
	iError = GetNormalisationGain(&fGain);
	if (iError) return iError;
	if (fGain != 1.0f)
	{
		oAmplifier.SetGain(fGain);
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oAmplifier));
		if (m_bPostProcessingLimiterEnabled)
		{
			oPeakLimiter.SetAttack(m_fPostProcessingLimiterAttack);
			oPeakLimiter.SetRelease(m_fPostProcessingLimiterRelease);
			oPeakLimiter.SetPeakLimit(m_fPostProcessingLimiterThreshold);
			oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oPeakLimiter));
		}
	}
	oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oLoudnessAnalyser));
	oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oPeakDetector));

	if (m_bEnableUpsampling)
	{
		oDestinationSampleRateConverter.SetBlockDuration(400);
		oDestinationSampleRateConverter.SetTargetSampleRate(m_iSampleRate);
		oDestinationSampleRateConverter.Set24bitOutput(false);
		oDestinationSampleRateConverter.SetDestination(dynamic_cast<IAudioDestination*>(&oWaveDestination));
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oDestinationSampleRateConverter));
	}
	else
	{
		oFilterList.push_back(dynamic_cast<IAudioFilter*>(&oWaveDestination));
	}

	oProcessing.SetFilters(oFilterList);

	oWaveSource.SetFileName(m_oInputFIle);
	oWaveDestination.SetFileName(m_oOutputFIle);
	iError = oProcessing.ProcessAllFrames();
	if (iError)	return iError;

	DWORD dwEndTime = timeGetTime();
	m_llNormalisationTime = dwEndTime - dwStartTime;

	iError = oLoudnessAnalyser.GetIntegratedLoudness(&m_fIntegratedLoudness);
	if (iError)	return iError;
	iError = oLoudnessAnalyser.GetTruePeak(&m_fTruepeak);
	if (iError)	return iError;
	iError =  oPeakDetector.GetPeakValue(&m_fSamplepeak);
	if (iError)	return iError;
	iError = oLoudnessAnalyser.GetMaxMomentaryLoudness(&m_fMaxMomentaryLoudness);
	if (iError)	return iError;
	iError = oLoudnessAnalyser.GetMaxShortTermLoudness(&m_fMaxShortTermLoudness);
	if (iError)	return iError;
	iError = oLoudnessAnalyser.GetLoudnessRange(&m_fLoudnessRange);
	if (iError)	return iError;

	//
	m_fPreProcessingLimiterMaxGainReduction = oPreProcPeakLimiter.GetMaxGainReduction();
	if (iError)	return iError;
	m_fPostProcessingLimiterMaxGainReduction = oPeakLimiter.GetMaxGainReduction();
	if (iError)	return iError;
	m_bAnalysed = true;

	return 0;
}

void CLoudnessNormaliser::Test()
{
	CBatchProcess oProcessing;
	CWAVReader oWaveSource;
	CWAVWriter oWaveDestination;
	CSRCSourceAdapter oSourceResampler;
	CSRCDestinationAdapter oDestinationAdapter;
	std::vector<IAudioFilter*> oFIlters;
	int iError;

	iError = oWaveSource.SetFileName(m_oInputFIle);
	iError = oWaveDestination.SetFileName(m_oOutputFIle);

	iError = oSourceResampler.SetSource(&oWaveSource);
	iError = oSourceResampler.SetTargetSampleRate(192000);
	iError = oSourceResampler.SetBlockDuration(400);
	oProcessing.SetSource(dynamic_cast<IAudioSource*>(&oSourceResampler));
	oProcessing.SetBlockDuration(400);
	oFIlters.push_back(&oWaveDestination);
	oProcessing.SetFilters(oFIlters);
	oProcessing.SetSource(dynamic_cast<IAudioSource*>(&oSourceResampler));
	oProcessing.SetBlockDuration(400);

	
	//oDestinationAdapter.SetTargetSampleRate(192000);
	//oDestinationAdapter.SetDestination(dynamic_cast<IAudioDestination*>(&oWaveDestination));
	//oFIlters.push_back(&oDestinationAdapter);
	//oProcessing.SetFilters(oFIlters);
	//oProcessing.SetSource(dynamic_cast<IAudioSource*>(&oWaveSource));
	//oProcessing.SetBlockDuration(400);


	iError = oProcessing.ProcessAllFrames();
}

int CLoudnessNormaliser::ProgessCallback(int status, unsigned long long processed, void* ctx)
{
	switch ((BatchProcessProgressStatus)status)
	{
	case BPS_END:
		printf("\r\n");
		break;
	case BPS_ERROR:
		printf("\r\nError\r\n");
		break;
	case BPS_PROCESS:
		CLoudnessNormaliser* pNormaliser = reinterpret_cast<CLoudnessNormaliser *>(ctx);
		long long timems = 0;

		if (pNormaliser != NULL)
		{
			long long effectiveSamplingrate = (pNormaliser->m_bEnableUpsampling) ? 192000 : pNormaliser->m_iSampleRate;
			timems = (processed *1000) / effectiveSamplingrate;
		}

		printf("Processed : %s (%lld frames)  \r",CStringTools::FormatTimeCode(timems).c_str(), processed);
		break;
	}

	return 0;
}
