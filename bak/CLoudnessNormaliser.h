#pragma once

#include <string>

typedef enum NormaliseOpTag {ANALYSE = 0, PROCESS} NormaliseOp;
typedef int (*NORMALISER_CALLBACK) (int status, unsigned long long processed, void* ctx);

class CNormaliserInternal;

class CLoudnessNormaliser
{	
	//analyser params
	bool m_bAnalysed;

	// normalisation target params
	float m_fTargetLoudness;
	float m_fTargetTruepeak;

	// preprocessing limiter params
	bool m_bPreProcessingAmplifierEnabled;
	bool m_bPreProcessingLimiterEnabled;

	// post processing params
	bool m_bPostProcessingLimiterEnabled;

	// files
	std::wstring m_oInputFIle;
	std::wstring m_oOutputFIle;

	//
	CNormaliserInternal* m_poInternal;

	//
	NORMALISER_CALLBACK m_pfProgressCallback;
	void* m_pvProgressCallbackContext;

	// test call back
	static int ProgessCallback(int status, unsigned long long processed, void* ctx);

public:
	CLoudnessNormaliser();
	~CLoudnessNormaliser();

	int SetInputFile(std::wstring oFile);
	int SetOutputFile(std::wstring oFile);

	int SetPreProcessingAmplifierEnabled(bool bAmpEnabled);
	int SetPreProcessingAmplifierGain(float fAmpGaindb);
	int SetPreProcessingLimiterEnabled(bool bLimiterEnabled);
	int SetPreProcessingLimiterPeak(float fLimiterPeakdBFS);
	int SetPreProcessingLimiterAttack(float fLimiterAttackms);
	int SetPreProcessingLimiterRelease(float fLimiterReleasems);

	int SetPostProcessingLimiterEnabled(bool bLimiterEnabled);
	int SetPostProcessingLimiterPeak(float fLimiterPeakdBFS);
	int SetPostProcessingLimiterAttack(float fLimiterAttackms);
	int SetPostProcessingLimiterRelease(float fLimiterReleasems);

	// Loudness Params
	int SetTargetLoudness(float fLUFS);
	int SetTargetLoudnessTruePeak(float fdBFS);

	//
	void SetProgressCallback(NORMALISER_CALLBACK pfCallback, void * pvContext );

	int GetMeasuredTruePeak(float* val);
	int GetMeasuredLoudnessRange(float* val);
	int GetMeasuredMaxMomentaryLoudness(float* val);
	int GetMeasuredMaxShorTermLoudness(float* val);
	int GetMeasuredIntegratedLoudness(float* val);

	// Loudness helper
	int IsLinearNormalisationPossible(bool * pbPossible);
	int GetNormalisationGain(float* pfGain);

	// processing
	int Analyse();
	int Normalise();


};

