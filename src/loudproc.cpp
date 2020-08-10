// loudproc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <locale>
#include <string>

#include "args.hxx"

#include "CLoudnessNormaliser.h"
#include "CBatchProcess.h"
#include "CAudioTools.h"
#include "CStringTools.h"

int ProgessCallback(int status, unsigned long long processed, void* ctx)
{
    switch ((BatchProcessProgressStatus)status)
    {
    case BPS_END:
        printf("\r\n", processed);
        break;
    case BPS_ERROR:
        printf("\r\nError\r\n", processed);
        break;
    case BPS_PROCESS:
        printf("frames processed: %ld \r", processed);
        break;
    }
    return 0;
}



int main(int argc, char** argv)
{
    args::ArgumentParser parser("Loudness normalisation Proof of concept using EBU R128 standard.","");
    
    args::Positional<std::string> inputFile(parser, "input", "The source file to process");    
    args::Positional<std::string> outputFile(parser, "output", "The target file to store processed audio. If not specified, only analysis is performed.");
    
    // TODO
    //args::Flag verbose(parser, "verbose", "Enable verbose mode", { 'v', "verbose" });
    args::Flag progress(parser, "progress", "Print progress.", { 'p', "progress" });    

    args::Flag limiterEnabled(parser, "limiter", "Enable peak limiter. If enabled, result loudness may vary from target loudness parameters.", { 'l', "limiter" });
    args::ValueFlag<int> attack(parser, "ms", "Peak limiter attack in ms (default 20ms)", { 'a', "attack" });
    args::ValueFlag<int> release(parser, "ms", "Peak limiter release in ms (default 20ms)", { 'r', "release" });
    args::ValueFlag<int> threshold(parser, "db", "Peak limiter threshold in dBFS (default -1dBFS), it is recommended to use the same value as target true peak.", { 't', "threshold" });

    args::Flag preProcAmpEnabled(parser, "amp", "Enable preprocessor amplifier", { 'M', "amplifier" });
    args::ValueFlag<double> preProcAmpGain(parser, "db", "preprocessor amplifier gain (default 0dB)", { 'G', "amplifier-gain" });

    args::Flag preProcLimiterEnabled(parser, "limiter", "Enable preprocessor peak limiter", { 'L', "preproc-limiter" });
    args::ValueFlag<int> preProcAttack(parser, "ms", "Preprocessor peak limiter attack in ms (default 20ms)", { 'A', "preproc-attack" });
    args::ValueFlag<int> preProcRelease(parser, "ms", "Preprocessor peak limiter release in ms (default 20ms)", { 'R', "preproc-release" });
    args::ValueFlag<int> preProcThreshold(parser, "db", "Preprocessor peak limiter threshold in dBFS (default -1dBFS)", { 'T', "preproc-threshold" });

    args::ValueFlag<double> targetLoudness(parser, "LUFS", "Target loudness (default -23LUFS)", { 'u', "loudness" });
    args::ValueFlag<double> targetMaxTruePeak(parser, "dbFS", "Target maximum true peak (default -1dBFS)", { 'k', "max-true-peak" });

    args::Flag monoAttenuation(parser, "mono", "Auto attenuate gain on mono signal if set (halves linear gain / approx -3 LUFS)", { 'm', "mono-attenuation" });
    args::Flag disableUpsampling(parser, "upsampling", "Disable 192kHz 64bit upsampling for faster processing but less accurate peak measurements", { 'd', "disable-upsampling" });

    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    if (inputFile.Get().empty())
    {
        std::cout << "No input file. Use -h for detailed help." << std::endl;
        return 0;
    }
  
    std::wstring in = CStringTools::StringToWideString(inputFile.Get());
    std::wstring out = CStringTools::StringToWideString(outputFile.Get());

    CLoudnessNormaliser oNormaliser;
    oNormaliser.SetInputFile(in);
    oNormaliser.SetOutputFile(out);

    oNormaliser.SetPreProcessing(
        preProcAmpEnabled.Get(),
        preProcAmpGain? (float)preProcAmpGain.Get():1.0f,
        preProcLimiterEnabled,
        preProcThreshold ? preProcThreshold.Get() : -1.0f,
        preProcAttack? preProcAttack.Get():20.0f,
        preProcRelease? preProcRelease.Get():20.0f
    );

    oNormaliser.SetPostProcessing(
        limiterEnabled,
        threshold ? threshold.Get() : -1.0f,
        attack?attack.Get():20.0f,
        release?release.Get() : 20.0f
    );

    oNormaliser.SetUpsamplingEnabled(!disableUpsampling.Get());

    if (targetLoudness)
        oNormaliser.SetTargetLoudness((float)targetLoudness.Get());
    if (targetMaxTruePeak)
        oNormaliser.SetTargetLoudnessTruePeak((float)targetMaxTruePeak.Get());
    if (progress) 
        oNormaliser.SetProgressCallback(&ProgessCallback, NULL);
    oNormaliser.SetMonoAttenution(monoAttenuation.Get());

    //
    //oNormaliser.Test();
    //return 0;
    //

    printf("Analysis:\r\n");
    int err;
    err = oNormaliser.Analyse();
    if (err != 0)
    {
        std::cout << "Analysis failed!" << std::endl;
        return err;
    }

    float val = 0;
    oNormaliser.GetMeasuredIntegratedLoudness(&val);
    printf("integrated loudness: %.2f LUFS\r\n", val);
    oNormaliser.GetMeasuredTruePeak(&val);
    printf("true peak: %.2f dBTP\r\n", linearTodBFS(val));
    oNormaliser.GetMeasuredSamplePeak(&val);
    printf("sample peak: %.2f dBFS\r\n", linearTodBFS(val));
    oNormaliser.GetMeasuredMaxMomentaryLoudness (&val);
    printf("max momentary loudness: %.2f dBFS\r\n", val);
    oNormaliser.GetMeasuredMaxShorTermLoudness (&val);
    printf("max short term loudness: %.2f dBFS\r\n", val);
    oNormaliser.GetMeasuredLoudnessRange(&val);
    printf("loudness range: %.2f LU\r\n", val);
    oNormaliser.GetNormalisationGain(&val);
    printf("gain to be applied: %.2f dB\r\n", linearTodBFS(val));

    bool linear;
    oNormaliser.IsLinearNormalisationPossible(&linear);
    printf("Linear normalisation possible: %s\r\n", (linear ? "yes" : "no"));

    if (outputFile.Get().empty()) return 0;

    if (!linear && !limiterEnabled)
    {
        printf("Normalisation not possible with provided parameters. Change target loudness/true peak or enable limiter.");
        return 1;
    }
    printf("\r\nNormalisation:\r\n");
    err = oNormaliser.Normalise();
    if (err != 0)
    {
        std::cout << "Normalisation failed!" << std::endl;
    }

    oNormaliser.GetMeasuredIntegratedLoudness(&val);
    printf("integrated loudness: %.2f LUFS\r\n", val);
    oNormaliser.GetMeasuredTruePeak(&val);
    printf("true peak: %.2f dBTP\r\n", linearTodBFS(val));
    oNormaliser.GetMeasuredSamplePeak(&val);
    printf("sample peak: %.2f dBFS\r\n", linearTodBFS(val));
    oNormaliser.GetMeasuredMaxMomentaryLoudness(&val);
    printf("max momentary loudness: %.2f LUFS\r\n", val);
    oNormaliser.GetMeasuredMaxShorTermLoudness(&val);
    printf("max short term loudness: %.2f LUFS\r\n", val);
    oNormaliser.GetMeasuredLoudnessRange(&val);
    printf("loudness range: %.2f LU\r\n", val);

    if (preProcLimiterEnabled.Get())
    {
        oNormaliser.GetPreProcessingStats(&val);
        printf("preproc max gain reduction: %.2f dB\r\n", val);
    }
    if (limiterEnabled.Get())
    {
        oNormaliser.GetPostProcessingStats(&val);
        printf("postproc max gain reduction: %.2f dB\r\n", val);
    }
    return 0;



}
