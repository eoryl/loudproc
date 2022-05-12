#include "CStringTools.h"

#if defined WIN32
#include <windows.h>
#else
#include <locale.h>
#endif

std::wstring CStringTools::StringToWideString(std::string str)
{
    if (str.empty())
    {
        return std::wstring();
    }
    size_t len = str.length() + 1;
    std::wstring ret = std::wstring(len, 0);
#if defined WIN32
    int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &str[0], str.size(), &ret[0], len);
    ret.resize(size);
#else
    size_t size = 0;
    _locale_t lc = _create_locale(LC_ALL, "en_US.UTF-8");
    errno_t retval = _mbstowcs_s_l(&size, &ret[0], len, &str[0], _TRUNCATE, lc);
    _free_locale(lc);
    ret.resize(size - 1);
#endif
    return ret;
}

std::string CStringTools::WidestringToString(std::wstring wstr)
{
    if (wstr.empty())
    {
        return std::string();
    }
#if defined WIN32
    int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), NULL, 0, NULL, NULL);
    std::string ret = std::string(size, 0);
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), &ret[0], size, NULL, NULL);
#else
    size_t size = 0;
    _locale_t lc = _create_locale(LC_ALL, "en_US.UTF-8");
    errno_t err = _wcstombs_s_l(&size, NULL, 0, &wstr[0], _TRUNCATE, lc);
    std::string ret = std::string(size, 0);
    err = _wcstombs_s_l(&size, &ret[0], size, &wstr[0], _TRUNCATE, lc);
    _free_locale(lc);
    ret.resize(size - 1);
#endif
    return ret;

}

std::string CStringTools::FormatTimeCode(long long llTimeMs)
{
    std::string res;
    long long llTimeSecs = llTimeMs / 1000;
    long long hours = hours = (llTimeSecs) / 3600;
    long long minutes = (llTimeSecs / 60) % 60;
    long long seconds = llTimeSecs % 60;
    long long millseconds = llTimeMs % 1000;

    size_t size = snprintf(NULL, 0, "%lld:%02lld:%02lld.%03lld", hours, minutes, seconds, millseconds);
    res.reserve(size + 1);
    res.resize(size);
    snprintf(&res[0], size + 1, "%lld:%02lld:%02lld.%03lld", hours, minutes, seconds, millseconds);

    return res;
}