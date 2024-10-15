#include "RemoteTimeTracker.h"

httplib::Server RemoteTimeTracker::_server;

bool RemoteTimeTracker::_running = false;

std::condition_variable RemoteTimeTracker::_has_stopped{};

std::mutex RemoteTimeTracker::_mutex{};

RemoteTimeTracker::RemoteTimeTracker()
{
	if (!RemoteTimeTracker::_running)
	{
		if (!RemoteTimeTracker::start())
		{
			Logger::log_error("Failed to start RemoteTimeTracker");
		}
		else
		{
			Logger::log_info("RemoteTimeTracker started");
		}
	}
}

RemoteTimeTracker::~RemoteTimeTracker()
{
	if (RemoteTimeTracker::_running)
	{
		if (!RemoteTimeTracker::stop())
		{
			Logger::log_error("Failed to stop RemoteTimeTracker");
		}
		else
		{
			Logger::log_info("RemoteTimeTracker stopped");
		}
	}
}

bool RemoteTimeTracker::start()
{
	std::unique_lock<std::mutex> lock(RemoteTimeTracker::_mutex);

	if (RemoteTimeTracker::_running)
	{
		Logger::log_warning("RemoteTimeTracker is already running");
		return true;
	}

	_server.Post("/", [](const httplib::Request& req, httplib::Response& res)
		{
			std::string body = req.body;
			if (_is_valid_body(body))
			{
				_handle_valid_body(body);
				res.set_content("VALID", "text/plain");
			}
			else
			{
				res.set_content("INVALID", "text/plain");
			}
		});

	_server.set_logger([](const httplib::Request& req, const httplib::Response& res)
		{
			DEBUG_LOG_LINE("Received request: " << req.method << " " << req.path << " " << res.status);
		});

	_server.set_error_handler([](const httplib::Request& req, httplib::Response& res)
		{
			Logger::log_error("Error: {} {} {}", req.method, req.path, res.status);
		});

	std::thread server_thread(_server_thread);

	server_thread.detach();

	return true;
}

bool RemoteTimeTracker::stop()
{
	std::unique_lock<std::mutex> lock(RemoteTimeTracker::_mutex);

	if (!RemoteTimeTracker::_running)
	{
		Logger::log_warning("RemoteTimeTracker is not running");
		return true;
	}

	_server.stop();

	RemoteTimeTracker::_has_stopped.wait(lock, [] { return !RemoteTimeTracker::_running; });

	return true;
}

void RemoteTimeTracker::_server_thread()
{
	RemoteTimeTracker::_running = true;

	Logger::log_info("RemoteTimeTracker server listening on port {}", REMOTE_TIME_TRACKER_PORT);

	_server.listen("localhost", REMOTE_TIME_TRACKER_PORT);

	RemoteTimeTracker::_running = false;

	RemoteTimeTracker::_has_stopped.notify_all();
}

bool RemoteTimeTracker::_is_valid_body(const std::string& body)
{
	if (body.empty() || body.size() < 5 || !body.starts_with("TTE:") || !body.ends_with(":"))
	{
		return false;
	}

	size_t count = std::count(body.begin(), body.end(), ':');
	if (count != 3)
	{
		return false;
	}

	return true;
}

void RemoteTimeTracker::_handle_valid_body(const std::string& body)
{
	std::string domain;
	std::string entity;

	size_t start = 4;
	size_t end = body.find(':', start);
	domain = body.substr(start, end - start);

	start = end + 1;
	end = body.find(':', start);
	entity = body.substr(start, end - start);

	std::time_t now = std::time(nullptr);
	std::tm local_time;
	localtime_s(&local_time, &now);

	Database::add_event(domain, entity, local_time);
}