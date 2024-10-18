#include "TTRFile.h"

TTRFile::TTRFile(const std::wstring& file_path)
	: _file_path(file_path), _file()
{
}

TTRFile::~TTRFile()
{
	if (_file.is_open())
	{
		_close();
	}
}

bool TTRFile::add_domain(const std::string& domain)
{
	if (!_file.is_open() && !_open())
	{
		return false;
	}

	if (!_read_info())
	{
		return false;
	}

	if (domain_exists(domain))
	{
		Logger::log_warning("Domain already exists: {}", domain);
		return false;
	}

	size_t required_size = domain.size() + 1;

	if (_offset_to_entities - _offset_to_domains_end < required_size)
	{
		// TODO: Implement block reallocation
		Logger::log_error("Not enough space to add domain: {}", domain);
		return false;
	}
	else
	{
		_file.seekp(_offset_to_domains_end);
		
		uint8_t len = domain.size();
		_file.write(reinterpret_cast<const char*>(&len), sizeof(len));

		_file.write(domain.c_str(), len);

		_offset_to_domains_end += required_size;

		_num_of_domains++;
		_file.seekg(_offset_to_current_domain);
		_file.write(reinterpret_cast<const char*>(&_num_of_domains), sizeof(_num_of_domains));

		_update_header();
	}

	_close();

	return true;
}

TTRFile::domain_id TTRFile::get_domain_id(const std::string& domain)
{
	if (!_file.is_open() && !_open())
	{
		return false;
	}

	if (!_read_info())
	{
		return false;
	}

	_file.seekg(_offset_to_current_domain);

	uint8_t num_of_domains;
	_file.read(reinterpret_cast<char*>(&num_of_domains), sizeof(num_of_domains));


	for (uint8_t i = 0; i < num_of_domains; i++)
	{
		uint8_t len;
		_file.read(reinterpret_cast<char*>(&len), sizeof(len));

		std::string name(len, '\0');
		_file.read(name.data(), len);

		if (name == domain)
		{
			return i;
		}
	}

	return -1;
}

bool TTRFile::domain_exists(const std::string& domain)
{
	return get_domain_id(domain) != (domain_id)-1;
}

bool TTRFile::add_entity(domain_id id, const std::string& entity)
{
	if (!_file.is_open() && !_open())
	{
		return false;
	}

	if (!_read_info())
	{
		return false;
	}

	if (entity_exists(id, entity))
	{
		Logger::log_warning("Entity already exists: {}", entity);
		return false;
	}

	_file.seekg(_offset_to_entities);

	uint16_t num_of_entities;
	_file.read(reinterpret_cast<char*>(&num_of_entities), sizeof(num_of_entities));

	num_of_entities++;

	_file.seekp(_offset_to_entities);
	_file.write(reinterpret_cast<const char*>(&num_of_entities), sizeof(num_of_entities));

	_file.seekp(0, std::ios::end);

	_file.write(reinterpret_cast<const char*>(&id), sizeof(id));

	uint8_t len = entity.size();
	_file.write(reinterpret_cast<const char*>(&len), sizeof(len));

	_file.write(entity.c_str(), len);

	return true;
}

TTRFile::entity_id TTRFile::get_entity_id(domain_id id, const std::string& entity)
{
	if (!_file.is_open() && !_open())
	{
		return false;
	}

	if (!_read_info())
	{
		return false;
	}

	_file.seekg(_offset_to_entities);

	uint16_t num_of_entities;
	_file.read(reinterpret_cast<char*>(&num_of_entities), sizeof(num_of_entities));

	for (uint16_t i = 0; i < num_of_entities; i++)
	{
		domain_id domain_id;
		_file.read(reinterpret_cast<char*>(&domain_id), sizeof(domain_id));

		uint8_t len;
		_file.read(reinterpret_cast<char*>(&len), sizeof(len));

		std::string name(len, '\0');
		_file.read(name.data(), len);

		if (domain_id == id && name == entity)
		{
			return i;
		}
	}

	return -1;
}

bool TTRFile::entity_exists(domain_id id, const std::string& entity)
{
	return get_entity_id(id, entity) != (entity_id)-1;
}

bool TTRFile::_open()
{
	if (!std::filesystem::exists(_file_path))
	{
		if (!_create())
		{
			return false;
		}
	}

	_file.open(_file_path, std::ios::in | std::ios::out | std::ios::binary);

	if (!_file.is_open())
	{
		Logger::log_error("Failed to open file: {}", (char*)_file_path.c_str());
		return false;
	}

	return true;
}

bool TTRFile::_close()
{
	_file.close();

	if (_file.is_open())
	{
		Logger::log_error("Failed to close file: {}", (char*)_file_path.c_str());
		return false;
	}

	return true;
}

bool TTRFile::_create()
{
	if (!std::filesystem::exists(_file_path))
	{
		std::filesystem::create_directories(std::filesystem::path(_file_path).parent_path());
	}

	_file.open(_file_path, std::ios::out | std::ios::binary);

	if (!_file.is_open())
	{
		char error_buffer[256];
		strerror_s(error_buffer, 256, errno);

		Logger::log_error("Failed to create file: {}", error_buffer);
		return false;
	}

	_offset_to_current_domain = 15;
	_offset_to_domains_end = 16;
	_num_of_domains = 0;

	_offset_to_entities = _offset_to_domains_end + DOMAINS_MIN_BLOCK_SIZE;
	_num_of_entities = 0;

	_update_header();

	_file.seekg(_offset_to_current_domain);
	_file.write(reinterpret_cast<const char*>(&_num_of_domains), sizeof(_num_of_domains));

	for (int i = 0; i < DOMAINS_MIN_BLOCK_SIZE; i++)
	{
		_file.write("#", 1);
	}

	_file.seekg(_offset_to_entities);

	_file.write(reinterpret_cast<const char*>(&_num_of_entities), sizeof(_num_of_entities));

	_close();

	return true;
}

bool TTRFile::_read_info()
{
	_file.seekg(0);

	char header[4];
	_file.read(header, 3);
	header[3] = '\0';

	if (std::string(header) != "TTR")
	{
		Logger::log_error("Invalid file header: {}", header);
		return false;
	}

	_file.read(reinterpret_cast<char*>(&_offset_to_current_domain), sizeof(_offset_to_current_domain));
	_file.read(reinterpret_cast<char*>(&_offset_to_domains_end), sizeof(_offset_to_domains_end));

	_file.read(reinterpret_cast<char*>(&_offset_to_entities), sizeof(_offset_to_entities));

	_file.seekg(_offset_to_current_domain);
	_file.read(reinterpret_cast<char*>(&_num_of_domains), sizeof(_num_of_domains));

	_file.seekg(_offset_to_entities);
	_file.read(reinterpret_cast<char*>(&_num_of_entities), sizeof(_num_of_entities));

	return true;
}

bool TTRFile::_update_header()
{
	_file.seekp(0);

	_file.write("TTR", 3);

	_file.write(reinterpret_cast<const char*>(&_offset_to_current_domain), sizeof(_offset_to_current_domain));
	_file.write(reinterpret_cast<const char*>(&_offset_to_domains_end), sizeof(_offset_to_domains_end));

	_file.write(reinterpret_cast<const char*>(&_offset_to_entities), sizeof(_offset_to_entities));

	return true;
}