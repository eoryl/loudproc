#pragma once
#include "IAudioFilter.h"
class CPeakDetector : public IAudioFilter
{
protected:
    float* m_pfPeaks;
    int m_iChannelCount;

public:
    CPeakDetector();
    ~CPeakDetector();

    int GetPeakValue(float* peak);
    int GetPeakValue(float* peak, int channel);
    bool HasClipped();

    // Inherited via IAudioFilter
    virtual int Init(int iSampleRate, int iChannels) override;
    virtual int Terminate() override;
    virtual int ProcessFrames(float * pfFrames, int iFrameCount) override;

    // Inherited via IAudioFilter
    virtual int GetDelay() override;
};

