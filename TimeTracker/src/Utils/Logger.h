#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <format>
#include <mutex>

#include <time.h>

#ifdef _DEBUG
#define DEBUG_LOG(x) std::cout << x
#define DEBUG_LOG_LINE(x) std::cout << x << std::endl
#else
#define DEBUG_LOG(x)
#define DEBUG_LOG_LINE(x)
#endif

enum class LogLevel
{
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR
};

class Logger
{
public:
	template <typename... Args>
	static void log(LogLevel level, const std::string& message, Args&&... args);

	template <typename... Args>
	static void log_info(const std::string& message, Args&&... args);

	template <typename... Args>
	static void log_warning(const std::string& message, Args&&... args);

	template <typename... Args>
	static void log_error(const std::string& message, Args&&... args);

	static void set_file_path(const std::wstring& file_path);

	static void set_log_level(LogLevel level);

	template <typename... Args>
	static void append_info(const std::string& message, Args&&... args);

private:
	static LogLevel _log_level;

	static std::wstring _file_path;

	static std::mutex _mutex;

private:
	static std::string _get_time_stamp();
};


template <typename... Args>
void Logger::log(LogLevel level, const std::string& message, Args&&... args)
{
	std::unique_lock<std::mutex> lock(_mutex);

	std::string formatted_message = std::vformat(message, std::make_format_args(args...));

	std::string log_level_string;
	switch (level)
	{
	case LogLevel::LOG_INFO:
		log_level_string = "INFO";
		break;
	case LogLevel::LOG_WARNING:
		log_level_string = "WARNING";
		break;
	case LogLevel::LOG_ERROR:
		log_level_string = "ERROR";
		break;
	}

	switch (_log_level)
	{
	case LogLevel::LOG_INFO:
		break;
	case LogLevel::LOG_WARNING:
		if (level == LogLevel::LOG_INFO)
			return;
		break;
	case LogLevel::LOG_ERROR:
		if (level == LogLevel::LOG_INFO || level == LogLevel::LOG_WARNING)
			return;
		break;
	}

	std::string log_message = _get_time_stamp() + " [" + log_level_string + "] " + formatted_message;

	DEBUG_LOG_LINE(log_message);

	if (!_file_path.empty())
	{
		if (!std::filesystem::exists(_file_path))
		{
			std::filesystem::create_directories(std::filesystem::path(_file_path).parent_path());
		}

		std::ofstream file(_file_path, std::ios::app);
		if (file.is_open())
		{
			file << log_message << std::endl;
			file.close();
		}
	}
}

template <typename... Args>
void Logger::log_info(const std::string& message, Args&&... args)
{
	log(LogLevel::LOG_INFO, message, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::log_warning(const std::string& message, Args&&... args)
{
	log(LogLevel::LOG_WARNING, message, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::log_error(const std::string& message, Args&&... args)
{
	log(LogLevel::LOG_ERROR, message, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::append_info(const std::string& message, Args&&... args)
{
	std::string formatted_message = std::vformat(message, std::make_format_args(args...));
	std::string line = std::string(25, ' ') + "- " + formatted_message;

	DEBUG_LOG_LINE(line);

	if (!_file_path.empty())
	{
		std::ofstream file(_file_path, std::ios::app);
		if (file.is_open())
		{
			file << line << std::endl;
			file.close();
		}
	}
}