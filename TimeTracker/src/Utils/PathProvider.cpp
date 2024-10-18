#include "PathProvider.h"

std::wstring PathProvider::_file_paths[3] = { L"", L"", L"" };

bool PathProvider::_has_been_set[3] = { false, false, false };

std::mutex PathProvider::_mutex;

std::wstring PathProvider::log_file_path()
{
	std::lock_guard<std::mutex> lock(PathProvider::_mutex);
	return PathProvider::_file_paths[_get_type_index(FileType::LOG)];
}

std::wstring PathProvider::ttr_file_path()
{
	std::lock_guard<std::mutex> lock(PathProvider::_mutex);
	return PathProvider::_file_paths[_get_type_index(FileType::TTR)];
}

std::wstring PathProvider::tte_file_path()
{
	std::lock_guard<std::mutex> lock(PathProvider::_mutex);
	return PathProvider::_file_paths[_get_type_index(FileType::TTE)];
}

bool PathProvider::set_file_path(const std::wstring& file_path, FileType file_type)
{
	std::lock_guard<std::mutex> lock(PathProvider::_mutex);

	if (file_path.empty())
	{
		Logger::log_error("File path cannot be empty");
		return false;
	}

	int index = _get_type_index(file_type);

	if (_has_been_set[index])
	{
		Logger::log_warning("File path has already been set");
		return false;
	}

	PathProvider::_file_paths[index] = file_path;
	_has_been_set[index] = true;

	return true;
}

bool PathProvider::use_default_location(DefaultLocation location)
{
	return (use_default_location(location, FileType::LOG) &&
			use_default_location(location, FileType::TTR) &&
			use_default_location(location, FileType::TTE));
}

bool PathProvider::use_default_location(DefaultLocation location, FileType file_type)
{
	std::lock_guard<std::mutex> lock(PathProvider::_mutex);

	int index = _get_type_index(file_type);

	if (_has_been_set[index])
	{
		Logger::log_warning("File path has already been set");
		return false;
	}

	std::wstring path;

	KNOWNFOLDERID folder_id;

	switch (location)
	{
		case DefaultLocation::APPDATA:
			folder_id = FOLDERID_RoamingAppData;
			break;
		case DefaultLocation::DESKTOP:
			folder_id = FOLDERID_Desktop;
			break;
		default:
			Logger::log_error("Invalid default location");
			return false;
	}

	wchar_t* path_ptr;

	if (SHGetKnownFolderPath(folder_id, 0, NULL, &path_ptr) != S_OK)
	{
		Logger::log_error("Failed to get default path");
		return false;
	}

	std::wstringstream ss;
	ss << path_ptr << L"\\TimeTracker\\";

	path = ss.str();

	CoTaskMemFree(path_ptr);

	switch (file_type)
	{
		case FileType::LOG:
			path += L"TimeTracker.log";
			break;
		case FileType::TTR:
			path += L"TimeTracker.ttr";
			break;
		case FileType::TTE:
			path += L"TimeTracker.tte";
			break;
		default:
			Logger::log_error("Invalid file type");
			return false;
	}

	PathProvider::_file_paths[index] = path;
	_has_been_set[index] = true;

	return true;
}

int PathProvider::_get_type_index(FileType path_type)
{
	switch (path_type)
	{
		case FileType::LOG:
			return 0;
		case FileType::TTR:
			return 1;
		case FileType::TTE:
			return 2;
		default:
			return -1;
	}
}