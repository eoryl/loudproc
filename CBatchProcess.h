/*
Description: A very basic audio processing framework
Author: BS

*/

#pragma once

#include <vector>

#include "IAudioSource.h"
#include "IAudioDestination.h"
#include "IAudioFilter.h"

typedef enum BatchProcessProgressStatusTag
{
	BPS_START =0,
	BPS_PROCESS,
	BPS_END,
	BPS_ERROR
} BatchProcessProgressStatus;

typedef int (*BATCH_PROCESS_CALLBACK) (int status, unsigned long long processed , void * ctx) ;

class CBatchProcess
{
	int m_iBlockDurationms;
	IAudioSource* m_poAudioSource;
	std::vector<IAudioFilter*> m_oFilterChain;
	BATCH_PROCESS_CALLBACK m_pfProgressCallback;
	void* m_pvProgressCallbackContext;
	long long m_qFramesProcessed;

public:
	CBatchProcess();
	~CBatchProcess();

	int SetBlockDuration(int iDurationms);
	int GetBlockDuration();

	void SetProgressCallback(BATCH_PROCESS_CALLBACK pfCallBack, void * pvContext);
	BATCH_PROCESS_CALLBACK SetProgressCallback();

	void SetSource(IAudioSource* poSource);
	void SetFilters(std::vector<IAudioFilter*> oFilters);

	int ProcessAllFrames();
};

