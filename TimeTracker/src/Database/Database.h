#pragma once

#include <string>

#include <time.h>

#include <mutex>

#include "TTRFile/TTRFileWriter.h"
#include "TTEFile/TTEFileWriter.h"

#include "../Utils/PathProvider.h"

class Database
{
public:
	static bool add_event(const std::string& domain, const std::string& entity);
	static bool add_event(const std::string& domain, const std::string& entity, std::tm time);

	static bool startup();
	static bool shutdown();

private:
	static TTRFileWriter* _registry;
	static TTEFileWriter* _events;

	static std::mutex _mutex;
};

