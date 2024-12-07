
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <time.h>

#include "src/Core/RemoteTimeTracker.h"
#include "src/Core/SystemTimeTracker.h"
#include "src/Core/ActivityMonitor.h"

#include "src/Database/DataBase.h"
#include "src/Database/TTRFile/TTRFileReader.h"
#include "src/Database/TTEFile/TTEFileReader.h"

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

	//Logger::set_file_path(PathProvider::log_file_path());
	Logger::set_log_level(LogLevel::LOG_INFO);

	TTRFileReader ttr_reader(PathProvider::ttr_file_path());
	//TTRFileWriter writer(PathProvider::ttr_file_path());

	TTEFileReader tte_reader(PathProvider::tte_file_path());
	//TTEFileWriter writer(PathProvider::tte_file_path());

	//writer.get_domain_id("test");

	for (auto event : tte_reader.events())
	{
		std::string entity = ttr_reader.get_entity(event.entity);
		std::string domain = ttr_reader.get_domain(event.entity);

		Logger::log_info("[{}.{}.{} - {}:{}:{}] ({}) {}",
			event.date.day, event.date.month, event.date.year,
			event.hour, event.minute, event.second,
			domain, entity);
	}

	/*tte_reader._align_buffer_to_start();
	tte_reader._populate_event_buffer();

	tte_reader._align_buffer_to_end();
	tte_reader._populate_event_buffer();*/

	/*Database::startup();

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
	}*/
}