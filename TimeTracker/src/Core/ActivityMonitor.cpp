#include "ActivityMonitor.h"

size_t ActivityMonitor::_timeout = 0;

UINT_PTR ActivityMonitor::_timer_id = 0;

bool ActivityMonitor::_is_active = false;

void ActivityMonitor::set_timeout(size_t timeout)
{
	_timeout = timeout;
}

bool ActivityMonitor::start()
{
	if (_timeout == 0)
	{
		Logger::log_warning("ActivityMonitor timeout is not set");
		return false;
	}

	return start(_timeout);
}

bool ActivityMonitor::start(size_t timeout)
{
	_is_active = true;

	if (_timer_id != 0)
	{
		Logger::log_warning("ActivityMonitor is already running");
		return true;
	}

	_timer_id = SetTimer(nullptr, 0, timeout, _check_for_inactivity);
	if (_timer_id == 0)
	{
		Logger::log_error("Failed to start ActivityMonitor");
		return false;
	}

	Logger::log_info("ActivityMonitor started");
	return true;
}

bool ActivityMonitor::stop()
{
	if (_timer_id == 0)
	{
		Logger::log_warning("ActivityMonitor is not running");
		return true;
	}

	if (!KillTimer(nullptr, _timer_id))
	{
		Logger::log_error("Failed to stop ActivityMonitor");
		return false;
	}

	_timer_id = 0;
	Logger::log_info("ActivityMonitor stopped");
	return true;
}

void CALLBACK ActivityMonitor::_check_for_inactivity(HWND hwnd, UINT message, UINT_PTR id_event, DWORD time)
{
	DWORD tick_count = GetTickCount();
	
	LASTINPUTINFO last_input_info;
	last_input_info.cbSize = sizeof(LASTINPUTINFO);

	if (!GetLastInputInfo(&last_input_info))
	{
		Logger::log_error("Failed to get last input info");
		return;
	}

	if (tick_count - last_input_info.dwTime > _timeout)
	{
		if (_is_active)
		{
			_is_active = false;
			_log_inactivity_start();
		}
	}
	else
	{
		if (!_is_active)
		{
			_is_active = true;
			_log_inactivity_end();
		}
	}
}

void ActivityMonitor::_log_inactivity_start()
{
	Logger::log_info("Inactivity started");
	Database::add_event("Activity", "Inactivity start");
}

void ActivityMonitor::_log_inactivity_end()
{
	Logger::log_info("Inactivity ended");
	Database::add_event("Activity", "Inactivity end");
}