#include "CAudioTools.h"
#include "macros.h"

void CAudioTools::BufferS16ToFloat(short* psSamples, float* pfSamples, int iSampleCount)
{
    for (int i = 0; i < iSampleCount; i++, psSamples++, pfSamples++)
    {

        // v1
        //*pfSamples = *psSamples;
        //*pfSamples /= S16MAX;
        
        //v1 alt
        //*pfSamples = *psSamples / 32767.0f;
        
        // v2
        *pfSamples = *psSamples;
        if (*pfSamples < 0) { *pfSamples = -*pfSamples;  *pfSamples /= S16MIN; }
        else *pfSamples /= S16MAX;
        

    }
}

void CAudioTools::BufferFloatToFS16(float* pfSamples, short* psSamples, int iSampleCount)
{
    for (int i = 0; i < iSampleCount; i++, psSamples++, pfSamples++)
    {
        //v1
        //*psSamples = roundf(S16MAX * (*pfSamples));
        
        // v2
        if (*psSamples < 0) *psSamples = S16MIN * (-*pfSamples);
        else *psSamples = S16MAX * (*pfSamples);
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

