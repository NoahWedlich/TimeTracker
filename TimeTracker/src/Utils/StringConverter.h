#pragma once

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class StringConverter
{
public:
	static std::string to_utf8(const std::wstring& str);
	static std::wstring to_utf16(const std::string& str);
};