#include "CWAVWriter.h"
#include "CAudioTools.h"
#include "errorcodes.h"
#include "macros.h"

CWAVWriter::CWAVWriter()
{
    m_iTotalBytesWritten = 0;
    ZeroMemory(&m_oWfx, sizeof(WAVEFORMATEX));
    m_oWfx.cbSize = sizeof(WAVEFORMATEX);
    m_pucBuffer = NULL;
}

CWAVWriter::~CWAVWriter()
{
    Terminate();
    SAFE_ARRAY_DELETE(m_pucBuffer);
}

int CWAVWriter::SetFileName(std::wstring oFile)
{
    m_oFileName = oFile;
    return 0;
}

std::wstring CWAVWriter::GetFileName()
{
    return m_oFileName;
}

int CWAVWriter::Init(int iSampleRate, int iChannels)
{
    if (m_oFileName.empty()) return AP_E_NOT_INITIALISED;
    m_iTotalBytesWritten = 0;

    int waveErr;

    // TODO: implement more formats later
    ZeroMemory(&m_oWfx, sizeof(WAVEFORMATEX));
    m_oWfx.cbSize = sizeof(WAVEFORMATEX);
    m_oWfx.nAvgBytesPerSec = (2 * iChannels * iSampleRate) ;
    m_oWfx.nBlockAlign = 2 * iChannels;
    m_oWfx.nChannels = iChannels;
    m_oWfx.nSamplesPerSec = iSampleRate;
    m_oWfx.wBitsPerSample = 16;
    m_oWfx.wFormatTag = WAVE_FORMAT_PCM;

    // FIXME: cast to TCHAR will only work in unicode mode
    waveErr = WaveCreateFile((TCHAR*) m_oFileName.c_str(), &m_hmmio, &m_oWfx, &m_mmcki, &m_mmckiRIFF);
    if (waveErr != 0)
    {
        OutputDebugString(TEXT("Error opening target file\r\n"));
        return AP_E_FAIL;
    }

    waveErr = WaveStartDataWrite(&m_hmmio, &m_mmcki, &m_mmioInf);
    if (waveErr != 0)
    {
        OutputDebugString(TEXT("Error writing target file\r\n"));
        return AP_E_FAIL;
    }

    return 0;
}

int CWAVWriter::Terminate()
{
    if (m_hmmio == NULL) return 0;

    int waveErr;
    waveErr = WaveCloseWriteFile(&m_hmmio, &m_mmcki, &m_mmckiRIFF, &m_mmioInf, m_iTotalBytesWritten);

    m_hmmio = NULL;

    if (waveErr) return AP_E_FAIL;
    else return 0;
}

int CWAVWriter::SetInternalBufferSize(int iByteCount)
{
    if (iByteCount < 0) return AP_E_FAIL;
    if (iByteCount == 0)
    {
        SAFE_ARRAY_DELETE(m_pucBuffer);
        return 0;
    }
    if (m_iBufferSize < iByteCount)
    {
        SAFE_ARRAY_DELETE(m_pucBuffer);
        // FIXME: see wavesource for alignment
        m_pucBuffer = new BYTE[iByteCount];
        m_iBufferSize = iByteCount;
    }
    if (m_pucBuffer == NULL)
    {
        m_iBufferSize = 0;
        return AP_E_OUT_OF_MEMORY;
    }

    return 0;

    return 0;
}


int CWAVWriter::ProcessFrames(float * pfFrames, int iFrameCount)
{
    if (m_hmmio == NULL) return AP_E_NOT_INITIALISED;

    int errorW;
    UINT iActualWrite = 0;
    SetInternalBufferSize(iFrameCount * m_oWfx.nBlockAlign);
    CAudioTools::BufferFloatToFS16(pfFrames, (short *) m_pucBuffer, iFrameCount * m_oWfx.nChannels  );

    errorW = WaveWriteFile(m_hmmio, iFrameCount * m_oWfx.nBlockAlign, (byte*)m_pucBuffer, &m_mmcki, &iActualWrite, &m_mmioInf);
    if (errorW != 0)
    {
        OutputDebugString(TEXT("Error writing to output file\r\n"));
        return AP_E_FAIL;
    }
    m_iTotalBytesWritten += iActualWrite;
    return 0;
}

int CWAVWriter::GetDelay()
{
    return 0;
}
