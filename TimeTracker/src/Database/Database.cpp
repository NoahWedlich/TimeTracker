#include "Database.h"

TTRFile* Database::_registry = nullptr;
TTEFile* Database::_events = nullptr;

std::mutex Database::_mutex{};

bool Database::add_event(const std::string& domain, const std::string& entity)
{
	time_t now = time(nullptr);
	std::tm local_time;
	localtime_s(&local_time, &now);

	return add_event(domain, entity, local_time);
}

bool Database::add_event(const std::string& domain, const std::string& entity, std::tm time)
{
	std::unique_lock<std::mutex> lock(_mutex);

	if (!_registry->domain_exists(domain))
	{
		Logger::log_info("Adding domain: {}", domain);
		_registry->add_domain(domain);
	}

	TTRFile::domain_id domain_id = _registry->get_domain_id(domain);

	if (!_registry->entity_exists(domain_id, entity))
	{
		Logger::log_info("Adding entity: {}", entity);
		_registry->add_entity(domain_id, entity);
	}

	TTRFile::entity_id entity_id = _registry->get_entity_id(domain_id, entity);

	TTEFile::Event last_event;
	if (_events->get_last_event(last_event))
	{
		if (last_event.entity == entity_id)
		{
			Logger::log_info("Skipping duplicate event: {}-{} {}:{}:{}", domain_id, entity_id, time.tm_hour, time.tm_min, time.tm_sec);
			return true;
		}
	}

	TTRFile::domain_id runtime_domain_id = _registry->get_domain_id("Runtime");
	TTRFile::entity_id startup_entity_id = _registry->get_entity_id(runtime_domain_id, "Startup");

	if (entity_id == startup_entity_id)
	{
		TTEFile::Date last_date;
		if (_events->get_last_date(last_date) && _events->get_last_event(last_event))
		{
			std::tm last_time;
			last_time.tm_year = last_date.year + 100;
			last_time.tm_mon = last_date.month - 1;
			last_time.tm_mday = last_date.day;
			last_time.tm_hour = last_event.hour;
			last_time.tm_min = last_event.minute;
			last_time.tm_sec = last_event.second;

			lock.unlock();
			Database::add_event("Runtime", "Shutdown", last_time);
			lock.lock();
		}
	}

	Logger::log_info("Adding event: {}-{} {}:{}:{}", domain_id, entity_id, time.tm_hour, time.tm_min, time.tm_sec);

	TTEFile::Date date(time.tm_year - 100, time.tm_mon + 1, time.tm_mday);
	TTEFile::Event event(entity_id, time.tm_hour, time.tm_min, time.tm_sec);

	return _events->add_event(date, event);
}

bool Database::startup()
{
	if (_registry == nullptr)
	{
		_registry = new TTRFile(PathProvider::ttr_file_path());
	}

	if (_events == nullptr)
	{
		_events = new TTEFile(PathProvider::tte_file_path());
	}

	time_t now = time(nullptr);
	std::tm local_time;
	localtime_s(&local_time, &now);

	add_event("Runtime", "Startup", local_time);

	return true;
}

bool Database::shutdown()
{
	time_t now = time(nullptr);
	std::tm local_time;
	localtime_s(&local_time, &now);

	add_event("Runtime", "Shutdown", local_time);

	if (_registry != nullptr)
	{
		delete _registry;
		_registry = nullptr;
	}

	if (_events != nullptr)
	{
		delete _events;
		_events = nullptr;
	}

	return true;
}