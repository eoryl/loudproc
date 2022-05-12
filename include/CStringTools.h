#pragma once

// string conversion function 
//
// std C++ <codecvt> std::converter works in C++11 but not in C++17 anymore
// this code
//#include <codecvt>
//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
//std::wstring s = converter.from_bytes(narrow);
// replaced by windows function

#include <string>



class CStringTools
{
public:
	static std::wstring StringToWideString(std::string str);
	static std::string WidestringToString(std::wstring wstr);
	static std::string FormatTimeCode(long long llTimeMs);
};