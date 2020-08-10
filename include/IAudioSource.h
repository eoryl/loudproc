#pragma once

/*
Interface for audio sources objects to be used as audio sources by an audio processing object.
*/
class IAudioSource
{

public:

	/*
	Will be called before processing commences
	*/
	virtual int Init() = 0;

	/*
	Will be called after processing completes
	*/
	virtual int Terminate() = 0;

	/*
	The source must Implement this method to indicate its sample rate and channel number
	It is expected that all sources/filters will work on interleaved PCM float samples array.
	Sample values should range between -1.0f and 1.0f
	out piSampleRate : Sample rate of the source
	out piChannels   : Number of channels
	*/
	virtual int GetFormat(int* piSampleRate, int* piChannels) = 0;
	/*
	Retrieves samples from the source.
	It is expected that all sources/filters will work on interleaved PCM float samples array.
	Sample values should range between -1.0f and 1.0f
	in pfFrames : Address to the buffer (array of float) to write the samples to. 
		the floats buffer size is iCount * channels (1 frame = 1 float sample per channel)
	in iCount : The size of buffer in frames (the buffer size in float sample is iCount * channel )
	out iActualCount : Number of frames actually written to the float buffer
	*/
	virtual int GetFrames(float* pfFrames, int iCount, int* iActualCount) = 0;
};
