#include "CBatchProcess.h"

#include <algorithm>

#include "CAudioTools.h"
#include "errorcodes.h"
#include "macros.h"



CBatchProcess::CBatchProcess()
{
	m_poAudioSource = NULL;
	m_iBlockDurationms = 400;
	m_pfProgressCallback = NULL;
	m_pvProgressCallbackContext = NULL;
	m_qFramesProcessed = 0;
}

CBatchProcess::~CBatchProcess()
{
}

int CBatchProcess::SetBlockDuration(int iDurationms)
{
	if (iDurationms <= 0) return AP_E_FAIL;
	m_iBlockDurationms = iDurationms;
	return 0;
}

int CBatchProcess::GetBlockDuration()
{
	return m_iBlockDurationms;
}

void CBatchProcess::SetProgressCallback(BATCH_PROCESS_CALLBACK pfCallBack, void * pvContext)
{
	m_pfProgressCallback = pfCallBack;
	m_pvProgressCallbackContext = pvContext;
}

BATCH_PROCESS_CALLBACK CBatchProcess::SetProgressCallback()
{
	return m_pfProgressCallback;
}

void CBatchProcess::SetSource(IAudioSource* poSource)
{
	m_poAudioSource = poSource;
}

void CBatchProcess::SetFilters(std::vector<IAudioFilter*> oFilters)
{
	m_oFilterChain = oFilters;

}

int CBatchProcess::ProcessAllFrames()
{
	if (
		(m_poAudioSource == NULL) 
		 || (m_oFilterChain.size() <=0)
		)
		return AP_E_NOT_INITIALISED;
	
	// TODO: make buffersize configurable
	int iError, iSampleRate = 0, iChannelCount = 0, iFrameBufferSize = 1920, iFramesRead = 0, iActualFrameCount = 0;
	float* pfFramesBuffer = NULL;
	long long qFrameCount, qTotalDelay= 0;
	int* piFiltersDelay = NULL;

	// open the source and get format
	m_qFramesProcessed = 0;
	iError = m_poAudioSource->Init();
	if (iError) goto CLEANUP_PROCESSALLFRAMES;

	iError = m_poAudioSource->GetFormat(&iSampleRate, &iChannelCount);
	if (iError) goto CLEANUP_PROCESSALLFRAMES;

	// avoid 32 bit int overflow for large buffer
	qFrameCount = iSampleRate;
	qFrameCount *= m_iBlockDurationms;
	qFrameCount /= 1000;
	iFrameBufferSize = qFrameCount;
	piFiltersDelay = new int[m_oFilterChain.size()];
	if (piFiltersDelay == NULL)
	{
		iError = AP_E_OUT_OF_MEMORY;
		goto CLEANUP_PROCESSALLFRAMES;
	}

	// signal init phase
	if (m_pfProgressCallback != NULL) m_pfProgressCallback(BPS_START, m_qFramesProcessed, m_pvProgressCallbackContext);

	int  iFilterIndex , * piThisFilterDelay;

	// initialise all the filters with the source sample rate and channel count, retrieve delay for each 
	// filter and calculate cumulated delay 
	for (iFilterIndex = 0, piThisFilterDelay = piFiltersDelay ; iFilterIndex < m_oFilterChain.size(); iFilterIndex++, piThisFilterDelay++)
	{
		iError = m_oFilterChain[iFilterIndex]->Init(iSampleRate, iChannelCount);
		if (iError) goto CLEANUP_PROCESSALLFRAMES;
		*piThisFilterDelay= m_oFilterChain[iFilterIndex]->GetDelay();
		qTotalDelay += *piThisFilterDelay;
	}

	// interleaved float samples buffer to pass through all filters
	pfFramesBuffer = new float[iSampleRate * iChannelCount];

	do
	{
		// get audio frames from the source
		iError = m_poAudioSource->GetFrames(pfFramesBuffer, iFrameBufferSize, &iFramesRead);
		if (iError) goto CLEANUP_PROCESSALLFRAMES;

		iActualFrameCount = iFramesRead;
		
		// if buffer is not full we are a the end of the stream, we still have to compensate filter induced delay
		// FIXME: implement a better method to signify end of stream in source
		if (iFramesRead < iFrameBufferSize)
		{
			// do zeropadding of the length of the delay if any
			if (qTotalDelay > 0)
			{
				int iThisPass = std::min(qTotalDelay, (long long) (iFrameBufferSize - iFramesRead));
				float* pfZeroPaddingStartPosition = pfFramesBuffer;
				pfZeroPaddingStartPosition += iFramesRead * iChannelCount;
				CAudioTools::ZeroBufferFloat(pfZeroPaddingStartPosition, iThisPass * iChannelCount);
				iActualFrameCount += iThisPass;
				qTotalDelay -= iThisPass;

			}

		}

		// if the buffer usage is 0 we don't even pass the buffer
		if (iActualFrameCount == 0) continue;

		// position in the buffer adjusted with the delay of the previous filters.
		// this is to allow us to we skip the delay 
		float* pfDelayCompensatedPostion = pfFramesBuffer;

		for (iFilterIndex = 0, piThisFilterDelay = piFiltersDelay; iFilterIndex < m_oFilterChain.size(); iFilterIndex++, piThisFilterDelay++)
		{
			// pass the buffer through the current filter
			//iError = m_oFilterSequence[iFilterIndex]->ProcessFrames(pFrames, iActualFrameCount);
			iError = m_oFilterChain[iFilterIndex]->ProcessFrames(pfDelayCompensatedPostion, iActualFrameCount);
			if (iError) goto CLEANUP_PROCESSALLFRAMES;
			// if the filter has a delay, we slide the window on the right.
			if (*piThisFilterDelay > 0)
			{
				// if no data left for the next filters we just go to the next buffer
				if (*piThisFilterDelay >= iActualFrameCount)
				{
					*piThisFilterDelay -= iActualFrameCount;
					continue;
				}
				// else we adjust the position in the buffer meaning following filters will process 
				// a partial buffer for their first buffer
				int iSkip = ((*piThisFilterDelay) * iChannelCount);
				pfDelayCompensatedPostion += iSkip;
				iActualFrameCount -= *piThisFilterDelay;
				*piThisFilterDelay = 0;
			}

		}
		m_qFramesProcessed += iActualFrameCount;
		// signal progress
		if (m_pfProgressCallback != NULL) m_pfProgressCallback(BPS_PROCESS, m_qFramesProcessed, m_pvProgressCallbackContext);
	}
	// when the buffer is empty we consider we reached end of processing
	while (iActualFrameCount > 0);

	// terminate all filters
	// FIXME: Move to cleanup phase and bypass error check ??
	for (unsigned int iFilterIndex = 0; iFilterIndex < m_oFilterChain.size(); iFilterIndex++)
	{
		iError = m_oFilterChain[iFilterIndex]->Terminate();
		if (iError) goto CLEANUP_PROCESSALLFRAMES;
	}

	// close source
	iError = m_poAudioSource->Terminate();
	if (iError) goto CLEANUP_PROCESSALLFRAMES;

	// ...

	// clean up
CLEANUP_PROCESSALLFRAMES:
	if (m_pfProgressCallback != NULL) m_pfProgressCallback( (iError ==0)? BPS_END: BPS_ERROR, m_qFramesProcessed, m_pvProgressCallbackContext);
	SAFE_ARRAY_DELETE(piFiltersDelay);
	SAFE_ARRAY_DELETE(pfFramesBuffer);
	return iError;
}
