#pragma once

#include <string>

typedef int (*NORMALISER_CALLBACK) (int status, unsigned long long processed, void* ctx);

class CLoudnessNormaliser
{
	//analyser params
	bool m_bAnalysed;
	float m_fLoudnessRange;
	float m_fMaxMomentaryLoudness;
	float m_fMaxShorTermLoudness;
	float m_fIntegratedLoudness;
	float m_fTruepeak;

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

	// post processing params
	bool m_bPostProcessingLimiterEnabled;
	float m_fPostProcessingLimiterAttack;
	float m_fPostProcessingLimiterRelease;
	float m_fPostProcessingLimiterThreshold;

	// format
	int m_iSampleRate;
	int m_iChannel;

	// files
	std::wstring m_oInputFIle;
	std::wstring m_oOutputFIle;

	//
	NORMALISER_CALLBACK m_pfProgressCallback;
	void* m_pvProgressCallbackContext;

	// call back
	static int ProgessCallback(int status, unsigned long long processed, void* ctx);

public:
	CLoudnessNormaliser();
	~CLoudnessNormaliser();

	int SetInputFile(std::wstring oFile);
	int SetOutputFile(std::wstring oFile);

	int SetPreProcessing(bool bAmpEnabled, float fAmpGaindb, bool bLimiterEnabled, float fLimiterPeakdBFS, float fLimiterAttackms, float fLimiterReleasems);
	int SetPostProcessing(bool bLimiterEnabled, float fLimiterPeakdBFS, float fLimiterAttackms, float fLimiterReleasems);

	// Loudness Params
	int SetTargetLoudness(float fLUFS);
	int SetTargetLoudnessTruePeak(float fdBFS);
	int SetMonoAttenution(bool bEnabled);

	//
	void SetProgressCallback(NORMALISER_CALLBACK pfCallback, void* pvContext);

	int GetMeasuredTruePeak(float* val);
	int GetMeasuredLoudnessRange(float* val);
	int GetMeasuredMaxMomentaryLoudness(float* val);
	int GetMeasuredMaxShorTermLoudness(float* val);
	int GetMeasuredIntegratedLoudness(float* val);

	// Loudness helper
	int IsLinearNormalisationPossible(bool* pbPossible);
	int GetNormalisationGain(float* pfGain);

	// processing
	int Analyse();
	int Normalise();


};

