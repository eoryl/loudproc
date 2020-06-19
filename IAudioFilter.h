#pragma once

#include "IAudioDestination.h"

/*
Interface for audio fklter objects to be used as to filter audio sequentially by an audio processing object.
*/
class IAudioFilter : public IAudioDestination
{
public:
	/*
	Return the delay of the filter in samples 
	*/
	virtual int GetDelay() = 0;

};
