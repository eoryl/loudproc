#pragma once

#include <math.h>

/** Get dbFS value of a a float sample (min value -1.0 to max value 1.0)
* @Returns dB full scale value
*/
#ifndef linearTodBFS
#define linearTodBFS(linear)  (20.0f* log10f(fabsf((linear))))
#endif 

/** Get dbFS and return a linear value of a flat sample (min value -1.0 to max value 1.0)
* @Returns lienar value
*/
#ifndef dBFSToLinear
#define dBFSToLinear(dbFS)   (powf(10, ((dbFS) / 20.0f)))
#endif 

class CAudioTools
{

public:
    static void BufferS16ToFloat(short* psSamples, float* pfSamples, int iSampleCount);

    static void BufferFloatToFS16(float* pfSamples, short* psSamples, int iSampleCount);

    static void ScaleBufferFloat(float* pfSamples, float fScale, int iSampleCount);

    static void ZeroBufferFloat(float* pfSamples, int iSampleCount);

};

