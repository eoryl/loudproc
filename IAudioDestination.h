#pragma once

class IAudioDestination
{
public:

	/*
	Will be called before processing commences
	in iSampleRate : Sample rate of the source
	in iChannels   : Number of channels
	*/
	virtual int Init(int iSampleRate, int iChannels) = 0;
	/*
	Will be called after processing completes
	*/
	virtual int Terminate() = 0;
	/*
	Tells the filter to process a .
	It is expected that all sources/filters will work on interleaved PCM float samples array.
	Sample values should range between -1.0f and 1.0f
	in pfFrames : Address to the buffer (array of float) of frames to process.
				   Restult is stored in place (the filtered samples are to be written in pfSamples)
	in iCount : Total number of samples per channels to process
	*/
	virtual int ProcessFrames(float* pfFrames, int iSampleCount) = 0;

};
