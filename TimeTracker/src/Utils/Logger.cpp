#include "Logger.h"

LogLevel Logger::_log_level = LogLevel::LOG_INFO;

std::string Logger::_file_path = "";

std::mutex Logger::_mutex{};

void Logger::set_file_path(const std::string& file_path)
{
	_file_path = file_path;

	DEBUG_LOG_LINE("Logger file path set to: " + _file_path);
}

void Logger::set_log_level(LogLevel level)
{
	_log_level = level;

	std::string log_level_str;
	switch (_log_level)
	{
	case LogLevel::LOG_INFO:
		log_level_str = "INFO";
		break;
	case LogLevel::LOG_WARNING:
		log_level_str = "WARNING";
		break;
	case LogLevel::LOG_ERROR:
		log_level_str = "ERROR";
		break;
	}

	DEBUG_LOG_LINE("Logger log level set to: " + log_level_str);
}

std::string Logger::_get_time_stamp()
{
	time_t raw_time;
	time(&raw_time);

	std::tm time_info;
	localtime_s(&time_info, &raw_time);

	char buffer[80];
	strftime(buffer, sizeof(buffer), "(%Y-%m-%d %H:%M:%S)", &time_info);

	return std::string(buffer);
}