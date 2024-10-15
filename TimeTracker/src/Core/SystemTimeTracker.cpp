#include "SystemTimeTracker.h"

bool SystemTimeTracker::_running = false;
bool SystemTimeTracker::_should_run = false;

std::condition_variable SystemTimeTracker::_has_stopped{};

std::mutex SystemTimeTracker::_mutex{};

SystemTimeTracker::SystemTimeTracker()
{
	if (!SystemTimeTracker::_running)
	{
		if(!SystemTimeTracker::start())
		{
			Logger::log_error("Failed to start SystemTimeTracker");
		}
		else
		{
			Logger::log_info("SystemTimeTracker started");
		}
	}
}

SystemTimeTracker::~SystemTimeTracker()
{
	if (SystemTimeTracker::_running)
	{
		if (!SystemTimeTracker::stop())
		{
			Logger::log_error("Failed to stop SystemTimeTracker");
		}
		else
		{
			Logger::log_info("SystemTimeTracker stopped");
		}
	}
}

bool SystemTimeTracker::start()
{
	std::unique_lock<std::mutex> lock(SystemTimeTracker::_mutex);

	if (SystemTimeTracker::_running)
	{
		Logger::log_warning("SystemTimeTracker is already running");
		return true;
	}

	SystemTimeTracker::_should_run = true;

	std::thread message_loop_thread(SystemTimeTracker::_message_loop);

	message_loop_thread.detach();

	return true;
}

bool SystemTimeTracker::stop()
{
	std::unique_lock<std::mutex> lock(SystemTimeTracker::_mutex);

	if (!SystemTimeTracker::_running)
	{
		Logger::log_warning("SystemTimeTracker is not running");
		return true;
	}

	SystemTimeTracker::_should_run = false;

	SystemTimeTracker::_has_stopped.wait(lock, [] { return !SystemTimeTracker::_running; });

	return true;
}

void SystemTimeTracker::_foreground_window_changed(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG id_object, LONG id_child, DWORD dw_event_thread, DWORD dwms_event_time)
{
	if (event == EVENT_SYSTEM_FOREGROUND && IsWindowVisible(hwnd))
	{
		DWORD process_id;
		GetWindowThreadProcessId(hwnd, &process_id);

		if (process_id == 0)
		{
			Logger::log_error("Failed to get process ID");
			return;
		}

		HANDLE process_handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, process_id);

		if (process_handle == nullptr)
		{
			Logger::log_error("Failed to open process");
			return;
		}

		char process_path[MAX_PATH];
		DWORD path_length = MAX_PATH;

		if (!QueryFullProcessImageNameA(process_handle, 0, process_path, &path_length))
		{
			Logger::log_error("Failed to get process path");
			CloseHandle(process_handle);
			return;
		}

		CloseHandle(process_handle);

		char* process_name = strrchr(process_path, '\\');

		if (process_name != nullptr)
		{
			process_name++;
		}
		else
		{
			process_name = process_path;
		}

		std::time_t current_time = std::time(nullptr);
		std::tm local_time;
		localtime_s(&local_time, &current_time);

		Database::add_event("System", process_name, local_time);
	}
}

bool SystemTimeTracker::_message_loop()
{
	HWINEVENTHOOK hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, SystemTimeTracker::_foreground_window_changed, 0, 0, WINEVENT_OUTOFCONTEXT);

	if (hook == nullptr)
	{
		Logger::log_error("Failed to set WinEventHook");
		return false;
	}

	SystemTimeTracker::_running = true;

	MSG msg;

	while (SystemTimeTracker::_should_run)
	{
		if (GetMessage(&msg, nullptr, 0, 0) <= 0)
		{
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWinEvent(hook);

	SystemTimeTracker::_running = false;

	SystemTimeTracker::_has_stopped.notify_one();

	return true;
}