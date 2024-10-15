#pragma once

#include <string>

#include <time.h>

#include <mutex>

#include "TTRFile.h"
#include "TTEFile.h"

class Database
{
public:
	static bool add_event(const std::string& domain, const std::string& entity);
	static bool add_event(const std::string& domain, const std::string& entity, std::tm time);

	static bool startup();
	static bool shutdown();

private:
	static std::string _file_path;

	static TTRFile _registry;
	static TTEFile _events;

	static std::mutex _mutex;
};

