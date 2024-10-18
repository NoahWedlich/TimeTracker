
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <time.h>

#include "src/Core/RemoteTimeTracker.h"
#include "src/Core/SystemTimeTracker.h"
#include "src/Core/ActivityMonitor.h"

#include "src/Database/DataBase.h"
#include "src/Database/TTRFile.h"

#include "src/Utils/Logger.h"
#include "src/Utils/PathProvider.h"

#ifdef _DEBUG
#define MAIN main()
#else
#define MAIN WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif

#define FIVE_MINUTES 300000

int MAIN
{
	PathProvider::use_default_location(PathProvider::DefaultLocation::APPDATA);

	Logger::set_file_path(PathProvider::log_file_path());
	Logger::set_log_level(LogLevel::LOG_INFO);

	Database::startup();

	/*TTRFile ttr_file("TimeTracker.ttr");

	for (auto domain : ttr_file.domains())
	{
		Logger::log_info("Domain: {}", domain.name);
	}*/

	try
	{
		SystemTimeTracker system_time_tracker;
		RemoteTimeTracker remote_time_tracker;

		ActivityMonitor::set_timeout(FIVE_MINUTES);
		ActivityMonitor::start();

		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Database::shutdown();
		return 0;
	}
	catch (const std::exception& e)
	{
		Logger::log_error("Unhandled exception: {}", e.what());
		Database::shutdown();
		return 1;
	}
	catch (...)
	{
		Logger::log_error("Unhandled exception");
		Database::shutdown();
		return 1;
	}
}