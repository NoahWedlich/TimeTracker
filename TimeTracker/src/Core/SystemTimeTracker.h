#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>

#include <Windows.h>

#include "../Utils/Logger.h"

#include "../Database/Database.h"

class SystemTimeTracker
{
public:
	SystemTimeTracker();
	~SystemTimeTracker();

	bool start();
	bool stop();

private:
	static void _foreground_window_changed(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD);

	static bool _message_loop();

private:
	static bool _running;
	static bool _should_run;
	static std::condition_variable _has_stopped;

	static std::mutex _mutex;
};

