#include "TTRFileReader.h"

TTRFileReader::TTRFileReader(const std::wstring& file_path)
	: _file_path(file_path), _file()
{
}

TTRFileReader::~TTRFileReader()
{
	if (_file.is_open())
	{
		_file.close();
	}
}

TTRFileReader::DomainIterator TTRFileReader::DomainRange::begin() const
{
	return _begin;
}

TTRFileReader::DomainIterator TTRFileReader::DomainRange::end() const
{
	return _end;
}

size_t TTRFileReader::DomainRange::size() const
{
	return _num_of_domains;
}

TTRFileReader::DomainRange::DomainRange(DomainIterator begin, DomainIterator end, uint8_t num_of_domains)
	: _begin(begin), _end(end), _num_of_domains(num_of_domains)
{
}

TTRFileReader::DomainRange TTRFileReader::domains()
{
	return domains(DomainFilter::empty());
}

TTRFileReader::DomainRange TTRFileReader::domains(DomainFilter filter)
{
	if (!_read_domains(filter))
	{
		Logger::append_info("Failed to read domains from file: {}", StringConverter::to_utf8(_file_path));
	}

	return DomainRange(_domains.begin(), _domains.end(), _num_of_domains);
}

bool TTRFileReader::domain_exists(const std::string& domain)
{
	return domain_exists(DomainFilter::create([&domain](domain_id, const std::string& d) {
		return d == domain;
	}));
}

TTRFileReader::domain_id TTRFileReader::get_domain_id(const std::string& domain)
{
	domain_id id = 0;

	walk_domains(
		DomainFilter::empty(),
		[&id, &domain](domain_id i, const std::string& d)
		{
			if (d == domain)
			{
				id = i;
				return false;
			}

			return true;
		}
	);

	return id;
}

uint8_t TTRFileReader::count_domains(DomainFilter filter)
{
	uint8_t count = 0;

	walk_domains(
		filter,
		[&count](domain_id, const std::string&)
		{
			count++;
			return true;
		}
	);

	return count;
}

bool TTRFileReader::domain_exists(DomainFilter filter)
{
	return count_domains(filter) > 0;
}

void TTRFileReader::walk_domains(DomainFilter filter, DomainWalker function)
{
	if (!_read_domains(filter))
	{
		Logger::append_info("Failed to walk domains from file: {}", StringConverter::to_utf8(_file_path));
		return;
	}

	for (domain_id i = 0; i < _num_of_domains; i++)
	{
		if (!function(i, _domains[i]))
		{
			break;
		}
	}
}

TTRFileReader::EntityIterator TTRFileReader::EntityRange::begin() const
{
	return _begin;
}

TTRFileReader::EntityIterator TTRFileReader::EntityRange::end() const
{
	return _end;
}

size_t TTRFileReader::EntityRange::size() const
{
	return _num_of_entities;
}

TTRFileReader::EntityRange::EntityRange(EntityIterator begin, EntityIterator end, uint16_t num_of_entities)
	: _begin(begin), _end(end), _num_of_entities(num_of_entities)
{
}

TTRFileReader::EntityRange TTRFileReader::entities()
{
	return entities(EntityFilter::empty());
}

TTRFileReader::EntityRange TTRFileReader::entities(EntityFilter filter)
{
	if (!_read_domains(DomainFilter::empty()))
	{
		Logger::append_info("Failed to read domains from file: {}", StringConverter::to_utf8(_file_path));
		return EntityRange(_entities.begin(), _entities.end(), _num_of_entities);
	}

	if (!_read_entities(filter))
	{
		Logger::append_info("Failed to read entities from file: {}", StringConverter::to_utf8(_file_path));
	}

	return EntityRange(_entities.begin(), _entities.end(), _num_of_entities);
}

bool TTRFileReader::entity_exists(const std::string& entity)
{
	return entity_exists(EntityFilter::create([&entity](entity_id, const Entity& e) {
		return e.name == entity;
	}));
}

bool TTRFileReader::entity_exists(domain_id id, const std::string& entity)
{
	return entity_exists(EntityFilter::create([id, &entity](entity_id, const Entity& e) {
		return e.domain_id == id && e.name == entity;
	}));
}

TTRFileReader::entity_id TTRFileReader::get_entity_id(domain_id d_id, const std::string& entity)
{
	entity_id id = 0;

	walk_entities(
		EntityFilter::empty(),
		[&id, d_id, &entity](entity_id i, const Entity& e)
		{
			if (e.domain_id == d_id && e.name == entity)
			{
				id = i;
				return false;
			}

			return true;
		}
	);

	return id;
}

TTRFileReader::entity_id TTRFileReader::get_entity_id(const std::string& entity, domain_id* id)
{
	entity_id e_id = 0;
	domain_id d_id = 0;

	walk_entities(
		EntityFilter::empty(),
		[&e_id, &d_id, &entity](entity_id i, const Entity& e)
		{
			if (e.name == entity)
			{
				e_id = i;
				d_id = e.domain_id;
				return false;
			}

			return true;
		}
	);

	if (id != nullptr)
	{
		*id = d_id;
	}

	return e_id;
}

std::string TTRFileReader::get_entity(entity_id id)
{
	std::string entity = "";

	walk_entities(
		EntityFilter::empty(),
		[id, &entity](entity_id current_id, const Entity& e)
		{
			if (id == current_id)
			{
				entity = e.name;
				return false;
			}
			return true;
		}
	);

	return entity;
}

std::string TTRFileReader::get_domain(entity_id id)
{
	std::string domain = "";

	walk_entities(
		EntityFilter::empty(),
		[id, &domain, this](entity_id current_id, const Entity& e)
		{
			if (id == current_id)
			{
				domain = _domains[e.domain_id];
				return false;
			}
			return true;
		}
	);

	return domain;
}

uint16_t TTRFileReader::count_entities(EntityFilter filter)
{
	uint16_t count = 0;

	walk_entities(
		filter,
		[&count](entity_id, const Entity&)
		{
			count++;
			return true;
		}
	);

	return count;
}

bool TTRFileReader::entity_exists(EntityFilter filter)
{
	return count_entities(filter) > 0;
}

void TTRFileReader::walk_entities(EntityFilter filter, EntityWalker function)
{
	if (!_read_domains(DomainFilter::empty()))
	{
		Logger::append_info("Failed to walk entities from file: {}", StringConverter::to_utf8(_file_path));
		return;
	}

	if (!_read_entities(filter))
	{
		Logger::append_info("Failed to walk entities from file: {}", StringConverter::to_utf8(_file_path));
		return;
	}

	for (entity_id i = 0; i < _num_of_entities; i++)
	{
		if (!function(i, _entities[i]))
		{
			break;
		}
	}
}

bool TTRFileReader::_open()
{
	if (!std::filesystem::exists(_file_path))
	{
		Logger::log_error("File does not exist: {}", StringConverter::to_utf8(_file_path));
		return false;
	}

	_file.open(_file_path, std::ios::in | std::ios::binary);

	if (!_file.is_open())
	{
		Logger::log_error("Failed to open file: {}", StringConverter::to_utf8(_file_path));
		return false;
	}

	return true;
}

bool TTRFileReader::_close()
{
	_file.close();

	if (_file.is_open())
	{
		Logger::log_error("Failed to close file: {}", StringConverter::to_utf8(_file_path));
		return false;
	}

	return true;
}

bool TTRFileReader::_read_header()
{
	if (!_open())
	{
		Logger::append_info("Failed to read header");
		return false;
	}

	_file.seekg(0, std::ios::beg);

	char magic[4];
	_file.read(magic, 3);
	magic[3] = '\0';

	if (std::strcmp(magic, "TTR") != 0)
	{
		Logger::log_error("Invalid file format: {}", StringConverter::to_utf8(_file_path));

		_close();

		return false;
	}

	_file.read(reinterpret_cast<char*>(&_header.offset_to_domains_start), sizeof(_header.offset_to_domains_start));
	_file.read(reinterpret_cast<char*>(&_header.offset_to_domains_end), sizeof(_header.offset_to_domains_end));
	_file.read(reinterpret_cast<char*>(&_header.offset_to_entities), sizeof(_header.offset_to_entities));

	return _close();
}

bool TTRFileReader::_read_domains(DomainFilter filter)
{
	if (!_read_header())
	{
		Logger::append_info("Failed to read domains");
		return false;
	}

	if (!_open())
	{
		Logger::append_info("Failed to read domains");
		return false;
	}

	_file.seekg(_header.offset_to_domains_start);

	_file.read(reinterpret_cast<char*>(&_num_of_domains), sizeof(_num_of_domains));

	_domains.clear();

	for (domain_id i = 0; i < _num_of_domains; i++)
	{
		uint8_t domain_len = 0;
		_file.read(reinterpret_cast<char*>(&domain_len), sizeof(domain_len));

		std::string domain = "";
		domain.resize(domain_len);
		_file.read(domain.data(), domain_len);

		if (filter(i, domain))
			_domains.push_back(domain);
	}

	return _close();
}

bool TTRFileReader::_read_entities(EntityFilter filter)
{
	if (!_read_header())
	{
		Logger::append_info("Failed to read entities");
		return false;
	}

	if (!_open())
	{
		Logger::append_info("Failed to read entities");
		return false;
	}

	_file.seekg(_header.offset_to_entities);

	_file.read(reinterpret_cast<char*>(&_num_of_entities), sizeof(_num_of_entities));

	_entities.clear();

	for (entity_id i = 0; i < _num_of_entities; i++)
	{
		Entity entity;

		_file.read(reinterpret_cast<char*>(&entity.domain_id), sizeof(entity.domain_id));

		uint8_t entity_len = 0;
		_file.read(reinterpret_cast<char*>(&entity_len), sizeof(entity_len));

		entity.name.resize(entity_len);
		_file.read(entity.name.data(), entity_len);

		if (filter(i, entity))
			_entities.push_back(entity);
	}

	return _close();
}