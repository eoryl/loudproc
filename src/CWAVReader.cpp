#include "CWAVReader.h"
#include "wave.h"
#include "CAudioTools.h"
#include "errorcodes.h"
#include "macros.h"

CWAVReader::CWAVReader()
{
    m_hmmio = NULL;
    m_pWfx = NULL;
    ZeroMemory(&m_mmcki, sizeof(m_mmcki));
    ZeroMemory(&m_mmckiRIFF, sizeof(m_mmckiRIFF));
    ZeroMemory(&m_mmioInf, sizeof(m_mmioInf));
    ZeroMemory(&m_oFormat, sizeof(m_oFormat));
    m_oFormat.cbSize = sizeof(WAVEFORMATEX);
    m_ulTotalBytesRead = 0;
    m_pucBuffer = NULL;
    m_iBufferSize = 0;

}

CWAVReader::~CWAVReader()
{
    Terminate();
    SAFE_ARRAY_DELETE(m_pucBuffer);
}

int CWAVReader::OpenFile()
{
    if (m_hmmio != NULL) 
        return 0; // already opened

    int waveErr;

    // FIXME: cast to TCHAR will only work in unicode mode
    waveErr = WaveOpenFile((TCHAR *) m_oFile.c_str() , &m_hmmio, &m_pWfx, &m_mmckiRIFF);
    m_ulTotalBytesRead = 0;

    if ((waveErr != 0) || (m_pWfx == NULL))
    {
        OutputDebugString(TEXT("Error loading source file\r\n"));
        return AP_E_FAIL;
    }

    m_oFormat = *m_pWfx;
    m_oFormat.cbSize = sizeof(WAVEFORMATEX);

    // TODO: add support for bit depth other than PCM 16 bits
    // 
    if (
        (m_oFormat.wBitsPerSample != 16) 
        // || (m_oFormat.wFormatTag != WAVE_FORMAT_PCM)
        )
    {
        CloseFile();
        return AP_E_NOT_SUPPORTED;
    }

    m_ulTotalBytesRead = 0;
    waveErr = WaveStartDataRead(&m_hmmio, &m_mmcki, &m_mmckiRIFF);
    if (waveErr != 0)
    {
        OutputDebugString(TEXT("Error reading source file\r\n"));
        CloseFile();
        return AP_E_FAIL;
    }

    // total size in bytes of data chunck
    //m_mmcki.cksize;

    return 0;

}

int CWAVReader::CloseFile()
{
    if ((m_hmmio== NULL) || (m_pWfx )) WaveCloseReadFile(&m_hmmio, &m_pWfx);
    //m_oFile = L"";
    m_hmmio = NULL;
    m_pWfx = NULL;
    return 0;
}

int CWAVReader::SetFileName(std::wstring oFile)
{
    m_oFile = oFile;
    return 0;
}

std::wstring CWAVReader::GetFileName()
{
    return std::wstring();
}

int CWAVReader::GetFileFormat(WAVEFORMATEX* pWfx)
{

    if (m_hmmio == NULL)
        OpenFile();

    if (m_pWfx == NULL)
        return AP_E_NOT_INITIALISED;
    *pWfx = *m_pWfx;
    //FIXME: WAVEFORMATEX can be larger than its struct size to handle "extended" info
    // implement later passing all extended data (need to implement another method to
    // send actual struct size  to caller before) for the moment we just copy the basics
    pWfx->cbSize = sizeof(WAVEFORMATEX);

    return 0;
}

int CWAVReader::GetFileDuration(unsigned long* pulDuration_ms)
{
    if (m_hmmio == NULL)
        OpenFile();

    if (m_pWfx == NULL)
        return AP_E_NOT_INITIALISED;
    // data chunk size in bytes
    float fDuration = m_mmcki.cksize;
    fDuration *= 1000;
    fDuration /= m_pWfx->nSamplesPerSec;
    *pulDuration_ms = fDuration;

    return 0;
}

int CWAVReader::Init()
{
    return OpenFile();
}

int CWAVReader::Terminate()
{
    //SAFE_ARRAY_DELETE(m_pucBuffer);
    //m_iBufferSize = 0;
    return CloseFile();
}

int CWAVReader::GetFormat(int* piSampleRate, int* piChannels)
{
    if (m_pWfx == NULL) return AP_E_NOT_INITIALISED;
    
    *piSampleRate = m_oFormat.nSamplesPerSec;
    *piChannels = m_oFormat.nChannels;
	return 0;
}

int CWAVReader::SetInternalBufferSize(int iByteCount)
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
        // FIXME: do something cleaner for memory alignment on at least 4 bytes or multiple
        // to ensure we can cast later to s16 or s32 being on the correct boundary
        // may be std::aligned_alloc or _aligned_malloc instead of new BYTE []
        // in theory malloc and new[]?? should be aligned on 8 bytes on 32 bit arch 
        // and 16 on 64bit so the cast of the buffer byte to s16 or s32 should still be safe
        m_pucBuffer = new BYTE[iByteCount];
        m_iBufferSize = iByteCount;
    }
    if (m_pucBuffer == NULL)
    {
        m_iBufferSize = 0;
        return AP_E_OUT_OF_MEMORY;
    }

    return 0;
}


int CWAVReader::GetFrames(float* pfFrames, int iCount, int* piActualCount)
{
    if ((m_pWfx == NULL) || (m_hmmio == NULL)) return AP_E_NOT_INITIALISED;
    if (iCount <= 0) 
    {
        *piActualCount = 0;
        return 0;
    }

    int iBufferSizeNeeded = m_oFormat.nBlockAlign * iCount;
    unsigned int  iBufferUsage = 0;
    int error = 0;

    error = SetInternalBufferSize(iBufferSizeNeeded);
    if (error) return error;

    error = WaveReadFile(m_hmmio, iBufferSizeNeeded, m_pucBuffer, &m_mmcki, &iBufferUsage);
    m_ulTotalBytesRead += iBufferUsage;

    if (error != 0)
    {
        OutputDebugString(TEXT("Error reading file\r\n"));
        return AP_E_FAIL;
    }
    if (iBufferUsage == 0)
    {        
        OutputDebugString(TEXT("Finished processing file\r\n"));
    }

    short* pBufferS = (short*)m_pucBuffer;
    int iSamplesInBuffer = iBufferUsage / (m_oFormat.wBitsPerSample / 8);
    int iFramesInBuffer = iBufferUsage / (m_oFormat.nBlockAlign);

    CAudioTools::BufferS16ToFloat(pBufferS, pfFrames, iSamplesInBuffer);

    *piActualCount = iFramesInBuffer;

	return 0;
}
