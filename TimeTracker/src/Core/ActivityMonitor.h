#pragma once

#include <Windows.h>

#include "../Database/Database.h"

#include "../Utils/Logger.h"

class ActivityMonitor
{
public:
	static void set_timeout(size_t timeout);

	static bool start();
	static bool start(size_t timeout);

	static bool stop();

private:
	static size_t _timeout;

	static UINT_PTR _timer_id;

	static bool _is_active;

private:
	static void CALLBACK _check_for_inactivity(HWND hwnd, UINT message, UINT_PTR id_event, DWORD time);

	static void _log_inactivity_start();
	static void _log_inactivity_end();
};

