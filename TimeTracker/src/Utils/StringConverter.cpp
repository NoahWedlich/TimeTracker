#include "StringConverter.h"

std::string StringConverter::to_utf8(const std::wstring& str)
{
	UINT code_page = CP_UTF8;
	int flags = 0;

	int size = WideCharToMultiByte(code_page, flags, str.c_str(), str.size(), nullptr, 0, nullptr, nullptr);

	std::string result(size, 0);

	WideCharToMultiByte(code_page, flags, str.c_str(), str.size(), result.data(), size, nullptr, nullptr);

	return result;
}

std::wstring StringConverter::to_utf16(const std::string& str)
{
	UINT code_page = CP_UTF8;
	int flags = 0;

	int size = MultiByteToWideChar(code_page, flags, str.c_str(), str.size(), nullptr, 0);

	std::wstring result(size, 0);

	MultiByteToWideChar(code_page, flags, str.c_str(), str.size(), result.data(), size);

	return result;
}