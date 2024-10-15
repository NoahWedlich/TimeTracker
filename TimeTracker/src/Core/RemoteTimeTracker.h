#pragma once

#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <time.h>

#include "../Database/Database.h"

#include "../Utils/httplib.h"
#include "../Utils/Logger.h"

/*
* Request body format:
* 
* TTE:<domain>:<entity>:
*/

#ifndef REMOTE_TIME_TRACKER_PORT
#define REMOTE_TIME_TRACKER_PORT 'R' + 'T' * 'T' // 7138
#endif

class RemoteTimeTracker
{
public:
	RemoteTimeTracker();
	~RemoteTimeTracker();

	bool start();
	bool stop();

private:
	static httplib::Server _server;

	static bool _running;
	static std::condition_variable _has_stopped;

	static std::mutex _mutex;

private:
	static void _server_thread();

private:
	static bool _is_valid_body(const std::string& body);
	static void _handle_valid_body(const std::string& body);
};

