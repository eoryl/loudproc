#include <stdlib.h>

#include "CSRCSourceAdapter.h"
#include "errorcodes.h"
#include "macros.h"

CSRCSourceAdapter::CSRCSourceAdapter()
{
	m_iSourceSampleRate = 48000;
	m_iChannelCount = 0;
	m_iBlockDurationms = 400;

	m_poSource = NULL;

	m_poResamplers = NULL;
	m_pfInBufferInterleaved = NULL;
	m_ppdInBuffers = NULL;
	m_ppdOutBuffers = NULL;
	m_iOutBufferSize = 0;
	m_iOutBufferPos = 0;
	m_iInBufferSize = 0;
	m_iInBufferInterleavedSize = 0;
	//m_iOutBufferSize = NULL;
	//m_iOutBufferPos = NULL;
	m_iTargetSampleRate = 192000;
}

CSRCSourceAdapter::~CSRCSourceAdapter()
{
	Terminate();
}

int CSRCSourceAdapter::SetSource(IAudioSource* poAudioSource)
{
	this->m_poSource = poAudioSource;
	return 0;
}

int CSRCSourceAdapter::SetTargetSampleRate(int iSampleRate)
{
	// limit target sample rates to multuples of 100 to support up to 44100
	if (iSampleRate % 100) return AP_E_NOT_SUPPORTED;
	m_iTargetSampleRate = iSampleRate;
	return 0;
}

// to support 44100 without convoluted buffer size adjustments enforce 10 ms multiples
int CSRCSourceAdapter::SetBlockDuration(int iBlockDurationMs)
{
	if (iBlockDurationMs % 10) return AP_E_NOT_SUPPORTED; 
	m_iBlockDurationms = iBlockDurationMs;
	return 0;
}



int CSRCSourceAdapter::Init()
{
	// if at least one of te internal values have been initialised
	// the object is already initialised
	if (m_poResamplers != NULL)
		return 0;

	int iError;

	if (m_poSource == NULL) return AP_E_NOT_INITIALISED;
	iError = m_poSource->Init();
	if (iError) return iError;
	iError = m_poSource->GetFormat(&m_iSourceSampleRate, &m_iChannelCount);
	if (iError) return iError;
	if ((m_iChannelCount <= 0) || (m_iBlockDurationms <= 0)) return AP_E_OUT_OF_RANGE;

	// allocate array addresses to double precision float sample buffers for each channel 
	// to be used as input by the sample rate converter
	m_ppdInBuffers = new double* [m_iChannelCount];
	for (int i = 0; i < m_iChannelCount; i++) m_ppdInBuffers[i] = NULL;
	// array of addresses of buffers containing the result of src for each channel
	m_ppdOutBuffers = new double *[m_iChannelCount];
	// size and positions in the output buffer
	m_iOutBufferSize = 0;
	m_iOutBufferPos = 0;
	// array of resampler pointer to hold address to the resampler objects for each channel
	m_poResamplers = new CR8BResampler [m_iChannelCount];

	// calculate the max size of input buffers
	long long llMaxBufferLength = m_iTargetSampleRate;
	llMaxBufferLength *= m_iBlockDurationms;
	llMaxBufferLength /= 1000;
	m_iInBufferSize = llMaxBufferLength;

	// internal buffer to read from the source
	m_iInBufferInterleavedSize = m_iInBufferSize;
	m_iInBufferInterleavedSize *= m_iChannelCount;
	m_pfInBufferInterleaved = new float[m_iInBufferInterleavedSize];

	// effectively creates the input double precision buffers and resamplers
	for (int i = 0; i < m_iChannelCount; i++)
	{
		m_ppdInBuffers[i] = new double[m_iInBufferSize];
		m_poResamplers[i] = r8b_create(m_iSourceSampleRate, m_iTargetSampleRate, m_iInBufferSize, 2.0, ER8BResamplerRes::r8brr24);
		if (m_poResamplers[i] == NULL) iError |= AP_E_NOT_INITIALISED;
	}
	if (iError) return iError;

	return 0;
}

int CSRCSourceAdapter::Terminate()
{
	if (m_poResamplers != NULL)
	{
		for (int i = 0; i < m_iChannelCount; i++)
		{
			if (m_poResamplers[i] != NULL) r8b_delete(m_poResamplers[i]);
		}
	}
	if (m_ppdInBuffers)
	{
		for (int i = 0; i < m_iChannelCount; i++)
		{
			SAFE_ARRAY_DELETE(m_ppdInBuffers[i]);
		}
		SAFE_ARRAY_DELETE(m_ppdInBuffers);
	}

	SAFE_ARRAY_DELETE(m_pfInBufferInterleaved);

	SAFE_ARRAY_DELETE(m_poResamplers);
	SAFE_ARRAY_DELETE(m_ppdOutBuffers);
	m_iOutBufferSize = 0;
	m_iOutBufferPos = 0;
	m_iInBufferSize = 0;
	m_iInBufferInterleavedSize = 0;

	return 0;
}

int CSRCSourceAdapter::GetFormat(int* piSampleRate, int* piChannels)
{
	int iError;
	iError = m_poSource->GetFormat(&m_iSourceSampleRate, &m_iChannelCount);
	if (iError) return iError;
	*piSampleRate = m_iTargetSampleRate;
	*piChannels = m_iChannelCount;

	return 0;
}



int CSRCSourceAdapter::GetFrames(float* pfFrames, int iCount, int* iActualCount)
{
	if (
		(m_poResamplers == NULL) ||
		(m_ppdOutBuffers == NULL) ||
		(m_pfInBufferInterleaved == NULL) ||
		(m_poSource == NULL)
	)
		return AP_E_NOT_INITIALISED;

	if (m_iChannelCount <= 0)
		return AP_E_OUT_OF_RANGE;

	// if we get more samples than initially planned
	if (iCount > m_iInBufferSize)
		return AP_E_OPERATION_NOT_POSSIBLE;

	int iFramesPos = 0;
	// empty what's left in the buffers first 
	while (iFramesPos < iCount)
	{
		// if the output buffers have been read or are empty
		// get more data from source and execute sample rate conversion
		if (m_iOutBufferPos >= m_iOutBufferSize)
		{
			int iSourceFramesActualCount = 0;
			int iError;

			iError = m_poSource->GetFrames(m_pfInBufferInterleaved, m_iInBufferSize, &iSourceFramesActualCount);
			if (iError) return iError;
			// we reached end of stream
			if (iSourceFramesActualCount <= 0)
			{
				*iActualCount = iFramesPos;
				return 0;
			}

			// copy content from single precision interleaved buffer to double precision in channel buffers
			for (int iFrame = 0; iFrame  < iSourceFramesActualCount; iFrame++)
			{
				for (int iChan = 0; iChan < m_iChannelCount; iChan++)
				{ 
					m_ppdInBuffers[iChan][iFrame] = m_pfInBufferInterleaved[ (iFrame * m_iChannelCount) + iChan];
				}
			}

			// perform sample rate conversion
			for (int iChan = 0; iChan < m_iChannelCount; iChan++)
			{
				// buffer size should (hopefully) be identical for all channels
				double* pdOutBuf = NULL;
				m_iOutBufferSize = r8b_process(m_poResamplers[iChan], m_ppdInBuffers[iChan], iSourceFramesActualCount, pdOutBuf);
				m_ppdOutBuffers[iChan] = pdOutBuf;
			}
			m_iOutBufferPos = 0;
		}

		// interleave samples from all channels into caller's buffer
		for (int iChan = 0; iChan < m_iChannelCount; iChan++)
		{
			pfFrames[(iFramesPos * m_iChannelCount) + iChan] = m_ppdOutBuffers[iChan][m_iOutBufferPos];
		}

		iFramesPos++;
		m_iOutBufferPos++;

	}

	*iActualCount = iFramesPos;
	return 0;
}
