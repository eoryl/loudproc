#include "CAudioTools.h"
#include "macros.h"

void CAudioTools::BufferS16ToFloat(short* psSamples, float* pfSamples, int iSampleCount)
{
    for (int i = 0; i < iSampleCount; i++, psSamples++, pfSamples++)
    {
        *pfSamples = *psSamples;
        *pfSamples /= S16MAX;
    }
}

void CAudioTools::BufferFloatToFS16(float* pfSamples, short* psSamples, int iSampleCount)
{
    for (int i = 0; i < iSampleCount; i++, psSamples++, pfSamples++)
    {
        *psSamples = roundf(S16MAX * (*pfSamples));
    }
}

void CAudioTools::ScaleBufferFloat(float* pfSamples, float fScale, int iSampleCount)
{
    for (int i = 0; i < iSampleCount; i++, pfSamples++)
    {
        *pfSamples *= fScale;
    }
}

void CAudioTools::ZeroBufferFloat(float* pfSamples, int iSampleCount)
{
    for (int i = 0; i < iSampleCount; i++, pfSamples++)
    {
        *pfSamples = 0.0f;
    }
}

