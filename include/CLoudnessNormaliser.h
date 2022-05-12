#pragma once

#include <string>

typedef int (*NORMALISER_CALLBACK) (int status, unsigned long long processed, void* ctx);

class CLoudnessNormaliser
{
	//analyser params
	bool m_bAnalysed;
	float m_fLoudnessRange;
	float m_fMaxMomentaryLoudness;
	float m_fMaxShortTermLoudness;
	float m_fIntegratedLoudness;
	float m_fTruepeak;
	float m_fSamplepeak;

	// normalisation params
	float m_fTargetLoudness;
	float m_fTargetTruepeak;
	bool m_bMonoAttenuation;

	// preprocessing limiter params
	bool m_bPreProcessingAmplifierEnabled;
	float m_fPreProcessingAmplifierGain;
	bool m_bPreProcessingLimiterEnabled;
	float m_fPreProcessingLimiterAttack;
	float m_fPreProcessingLimiterRelease;
	float m_fPreProcessingLimiterThreshold;
	float m_fPreProcessingLimiterMaxGainReduction;

	// post processing params
	bool m_bPostProcessingLimiterEnabled;
	float m_fPostProcessingLimiterAttack;
	float m_fPostProcessingLimiterRelease;
	float m_fPostProcessingLimiterThreshold;
	float m_fPostProcessingLimiterMaxGainReduction;

	// format
	int m_iSampleRate;
	int m_iChannel;

	// upsampling
	bool m_bEnableUpsampling;

	// files
	std::wstring m_oInputFIle;
	std::wstring m_oOutputFIle;
		
	// stats
	long long m_llAnalysisTime;
	long long m_llNormalisationTime;

	//
	NORMALISER_CALLBACK m_pfProgressCallback;
	void* m_pvProgressCallbackContext;


public:
	CLoudnessNormaliser();
	virtual ~CLoudnessNormaliser();

	int SetInputFile(std::wstring oFile);
	int SetOutputFile(std::wstring oFile);

	int SetPreProcessing(bool bAmpEnabled, float fAmpGaindb, bool bLimiterEnabled, float fLimiterPeakdBFS, float fLimiterAttackms, float fLimiterReleasems);
	int SetPostProcessing(bool bLimiterEnabled, float fLimiterPeakdBFS, float fLimiterAttackms, float fLimiterReleasems);
	int SetUpsamplingEnabled(bool bEnabled);

	// Loudness Params
	int SetTargetLoudness(float fLUFS);
	int SetTargetLoudnessTruePeak(float fdBFS);
	int SetMonoAttenution(bool bEnabled);

	//
	void SetProgressCallback(NORMALISER_CALLBACK pfCallback, void* pvContext);

	int GetMeasuredTruePeak(float* val);
	int GetMeasuredSamplePeak(float* val);
	int GetMeasuredLoudnessRange(float* val);
	int GetMeasuredMaxMomentaryLoudness(float* val);
	int GetMeasuredMaxShorTermLoudness(float* val);
	int GetMeasuredIntegratedLoudness(float* val);

	int GetPreProcessingStats(float* pfLimiterMaxGainReductiondB);
	int GetPostProcessingStats(float* pfLimiterMaxGainReductiondB);
	int GetAnalysisTime(long long* pllAnalysisTime);
	int GetNormalisationTime(long long* pllNormalisationTime);
	int GetFileDuration(long long* pllFileDuration);

	// Loudness helper

	// returns true if it is possible to apply a fixed gain to reach target programme loudnes
	// withouht exeeding maxTP
	// returns false, result will be out of range and may be corrected using
	// peak limiter as post processing (which may in turn invalidate programme loudness measurement)
	int IsLinearNormalisationPossible(bool* pbPossible);

	// return gain to attain desired programme loudness regardless of of maxTP and max momentary/short term loudness
	int GetNormalisationGain(float* pfGain);

	// attempts to calculate a fixed gain to achieve a result that is within tolerance
	// programme loundess +/- tolerance
	// tp < maxTP
	// Maximum Momentary Loudness < maxML OR Maximum Short-term Loudness < maxSL
	int GetConstrainedNormalisationGain(float* pfGain);

	// processing
	int Analyse();
	int Normalise();

	//
	void Test();

	// call back
	static int ProgessCallback(int status, unsigned long long processed, void* ctx);

};

