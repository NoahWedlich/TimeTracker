#pragma once

#include <string>
#include <mutex>
#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj_core.h>

#include "Logger.h"

class PathProvider
{
public:
	static std::wstring log_file_path();
	static std::wstring ttr_file_path();
	static std::wstring tte_file_path();

public:
	enum class FileType
	{
		LOG,
		TTR,
		TTE
	};

	enum class DefaultLocation
	{
		APPDATA,
		DESKTOP
	};

public:
	static bool use_default_location(DefaultLocation location);

	static bool set_file_path(const std::wstring& file_path, FileType file_type);
	static bool use_default_location(DefaultLocation location, FileType file_type);

private:
	static std::wstring _file_paths[3]; // [LOG, TTR, TTE]

	static bool _has_been_set[3]; // [LOG, TTR, TTE]

	static std::mutex _mutex;

private:
	static int _get_type_index(FileType path_type);
};