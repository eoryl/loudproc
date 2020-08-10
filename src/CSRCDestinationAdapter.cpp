#include "CSRCDestinationAdapter.h"
#include <stdio.h>

#include "errorcodes.h"
#include "macros.h"
#include <algorithm>


CSRCDestinationAdapter::CSRCDestinationAdapter()
{
	m_iSourceSampleRate = 48000;
	m_iTargetSampleRate = 48000;
	m_iChannelCount = 0;
	m_iBlockDurationms = 400;
	m_b24bit = false;

	m_poDestination = NULL;
	m_poResamplers = NULL;
	m_ppdInBuffers = NULL;

	m_iOutBufferSize = 0;

	m_ppdOutBuffers = NULL;
	m_iOutBufferSize = 0;

	m_pfOutBufferInterleaved = NULL;
	m_iOutBufferInterleavedSize = 0;

}

CSRCDestinationAdapter::~CSRCDestinationAdapter()
{
	Terminate();
}

int CSRCDestinationAdapter::SetDestination(IAudioDestination* poDestination)
{
	m_poDestination = poDestination;
	return 0;
}

int CSRCDestinationAdapter::SetTargetSampleRate(int iSampleRate)
{
	// limit target sample rates to multuples of 100 to support up to 44100
	if (iSampleRate % 100) return AP_E_NOT_SUPPORTED;
	m_iTargetSampleRate = iSampleRate;
	return 0;
}

int CSRCDestinationAdapter::SetBlockDuration(int iDurationMs)
{
	m_iBlockDurationms = iDurationMs;
	return 0;
}

int CSRCDestinationAdapter::Set24bitOutput(bool b24)
{
	m_b24bit = b24;
	return 0;
}

int CSRCDestinationAdapter::Init(int iSampleRate, int iChannels)
{
	// if at least one of te internal values have been initialised
	// the object is already initialised
	if (m_poResamplers != NULL)
		return 0;

	int iError = 0;

	if (m_poDestination != NULL)
	{
		iError = m_poDestination->Init(m_iTargetSampleRate, iChannels);
		if (iError) return iError;
	}
	
	m_iChannelCount = iChannels;

	if ((m_iChannelCount <= 0) || (m_iBlockDurationms <= 0)) return AP_E_OUT_OF_RANGE;

	// allocate array addresses to double precision float sample buffers for each channel 
	// to be used as input by the sample rate converter
	m_ppdInBuffers = new double* [m_iChannelCount];
	for (int i = 0; i < m_iChannelCount; i++) m_ppdInBuffers[i] = NULL;
	// array of addresses of buffers containing the result of src for each channel
	m_ppdOutBuffers = new double* [m_iChannelCount];
	// size of the output buffer
	m_iOutBufferSize = 0;
	// array of resampler pointer to hold address to the resampler objects for each channel
	m_poResamplers = new CR8BResampler[m_iChannelCount];

	// calculate the max size of input buffers
	long long llMaxBufferLength = iSampleRate;
	llMaxBufferLength *= m_iBlockDurationms;
	llMaxBufferLength /= 1000;

	m_iOutBufferSize = llMaxBufferLength;
	m_iOutBufferInterleavedSize = m_iOutBufferSize * m_iChannelCount;
	m_pfOutBufferInterleaved = new float[m_iOutBufferInterleavedSize];
	m_iSourceSampleRate = iSampleRate;

	// effectively creates the input double precision buffers and resamplers
	for (int i = 0; i < m_iChannelCount; i++)
	{
		m_ppdInBuffers[i] = new double[m_iOutBufferSize];
		m_poResamplers[i] = r8b_create(m_iSourceSampleRate, m_iTargetSampleRate, m_iOutBufferSize, 2.0, (m_b24bit?ER8BResamplerRes::r8brr24:ER8BResamplerRes::r8brr16));
		if (m_poResamplers[i] == NULL) iError |= AP_E_NOT_INITIALISED;
	}
	if (iError) return iError;

	return 0;
}

int CSRCDestinationAdapter::Terminate()
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

	SAFE_ARRAY_DELETE(m_poResamplers);
	SAFE_ARRAY_DELETE(m_ppdOutBuffers);
	SAFE_ARRAY_DELETE(m_pfOutBufferInterleaved);
	m_iOutBufferInterleavedSize = 0;
	m_iOutBufferSize = 0;

	return 0;
}

int CSRCDestinationAdapter::ProcessFrames(float* pfFrames, int iFrameCount)
{
	// is initilised ?
	if (m_poResamplers == NULL)
		return AP_E_NOT_INITIALISED;

	// something to process ?
	if ((iFrameCount <= 0) || (pfFrames == NULL))
		return 0;

	// are we getting more than what we initialised the resamplers for ?
	if (iFrameCount > m_iOutBufferSize)
		return AP_E_OUT_OF_RANGE;
	
	// split incoming buffer into distinct channels
	int iOutBufferUsage = 0;
	for (int iChannel = 0; iChannel < m_iChannelCount; iChannel++)
	{
		// get channel buffer
		for (int iFrame = 0; iFrame < iFrameCount; iFrame++)
		{
			m_ppdInBuffers[iChannel][iFrame] = pfFrames[iFrame * m_iChannelCount + iChannel];
		}

		// resample channel
		double* pdOutBuf = NULL;
		int iRes;
		// FIXME: here we expect output buffersize is the same for all channels (since in buffers have the same sizes)
		iRes = r8b_process(m_poResamplers[iChannel], m_ppdInBuffers[iChannel], iFrameCount, pdOutBuf);
		if (iChannel == 0) iOutBufferUsage = iRes;
		else iOutBufferUsage = std::min(iOutBufferUsage, iRes);
		m_ppdOutBuffers[iChannel] = pdOutBuf;
	}


	// extends the intermediate interleaved output buffer if needed
	if (m_iOutBufferInterleavedSize < iOutBufferUsage * m_iChannelCount)
	{
		SAFE_ARRAY_DELETE(m_pfOutBufferInterleaved);
		m_iOutBufferInterleavedSize = iOutBufferUsage * m_iChannelCount;
		m_pfOutBufferInterleaved = new float[m_iOutBufferInterleavedSize];
	}

	// fill the intermediate interleaved output buffer 
	for (int iChannel = 0; iChannel < m_iChannelCount; iChannel++)
	{
		for (int iSample = 0; iSample < iOutBufferUsage; iSample++)
		{
			m_pfOutBufferInterleaved[iSample * m_iChannelCount + iChannel] = m_ppdOutBuffers[iChannel][iSample];
		}
	}

	// passes resampled buffer to final destination
	if (m_poDestination != NULL)
	{
		int iError;
		iError = m_poDestination->ProcessFrames(m_pfOutBufferInterleaved, iOutBufferUsage);
		if (iError) return iError;
	}

	return 0;
}

int CSRCDestinationAdapter::GetDelay()
{
	return 0;
}
