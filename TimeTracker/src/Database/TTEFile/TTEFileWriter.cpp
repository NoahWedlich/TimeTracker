#include "TTEFileWriter.h"

// Date

TTEFileWriter::Date::Date()
	: year(0), month(0), day(0)
{
}

TTEFileWriter::Date::Date(uint8_t year, uint8_t month, uint8_t day)
	: year(year), month(month), day(day)
{
	if (!check_validity())
	{
		Logger::append_info("Unable to create date: {}-{}-{}", year, month, day);

		year = 0;
		month = 0;
		day = 0;
	}
}

void TTEFileWriter::Date::encode(encoded_date& date) const
{
	if (!check_validity())
	{
		Logger::append_info("Unable to encode date: {}-{}-{}", year, month, day);

		date = 0;
		return;
	}

	date = (uint16_t(year) << 9) | (uint16_t(month) << 5) | uint16_t(day);
}

const TTEFileWriter::Date TTEFileWriter::Date::decode(const encoded_date& date)
{
	Date decoded_date;

	decoded_date.year = uint8_t((date >> 9) & 0x7F);
	decoded_date.month = uint8_t((date >> 5) & 0x0F);
	decoded_date.day = uint8_t(date & 0x1F);

	if (!decoded_date.check_validity())
	{
		Logger::append_info("Unable to decode date: {}-{}-{}", decoded_date.year, decoded_date.month, decoded_date.day);

		decoded_date.year = 0;
		decoded_date.month = 0;
		decoded_date.day = 0;
	}

	return decoded_date;
}

bool TTEFileWriter::Date::operator==(const Date& other) const
{
	return year == other.year && month == other.month && day == other.day;
}

bool TTEFileWriter::Date::check_validity() const
{
	if (year < 0 || year > 127)
	{
		Logger::log_error("Invalid year: {}", year);
		return false;
	}

	if (month < 1 || month > 12)
	{
		Logger::log_error("Invalid month: {}", month);
		return false;
	}

	if (day < 1 || day > 31)
	{
		Logger::log_error("Invalid day: {}", day);
		return false;
	}

	return true;
}

// Event

TTEFileWriter::Event::Event()
	: entity(0), hour(0), minute(0), second(0)
{
}

TTEFileWriter::Event::Event(entity_id entity, uint8_t hour, uint8_t minute, uint8_t second)
	: entity(entity), hour(hour), minute(minute), second(second)
{
	if (!check_validity())
	{
		Logger::append_info("Unable to create event: {} {}:{}:{}", entity, hour, minute, second);

		entity = 0;
		hour = 0;
		minute = 0;
		second = 0;
	}
}

void TTEFileWriter::Event::encode(encoded_event& event) const
{
	if (!check_validity())
	{
		Logger::append_info("Unable to encode event: {} {}:{}:{}", entity, hour, minute, second);

		event = 0;
		return;
	}

	event = (uint32_t(entity) << 17) | (uint32_t(hour) << 12) | (uint32_t(minute) << 6) | uint32_t(second);
}

const TTEFileWriter::Event TTEFileWriter::Event::decode(const encoded_event& event)
{
	Event decoded_event;

	decoded_event.entity = entity_id((event >> 17) & 0x7FFF);
	decoded_event.hour = uint8_t((event >> 12) & 0x1F);
	decoded_event.minute = uint8_t((event >> 6) & 0x3F);
	decoded_event.second = uint8_t(event & 0x3F);

	if (!decoded_event.check_validity())
	{
		Logger::append_info("Unable to decode event: {} {}:{}:{}", decoded_event.entity, decoded_event.hour, decoded_event.minute, decoded_event.second);

		decoded_event.entity = 0;
		decoded_event.hour = 0;
		decoded_event.minute = 0;
		decoded_event.second = 0;
	}

	return decoded_event;
}

bool TTEFileWriter::Event::check_validity() const
{
	if (entity < 0 || entity > 0x7FFF)
	{
		Logger::log_error("Invalid entity: {}", entity);
		return false;
	}

	if (hour < 0 || hour > 23)
	{
		Logger::log_error("Invalid hour: {}", hour);
		return false;
	}

	if (minute < 0 || minute > 59)
	{
		Logger::log_error("Invalid minute: {}", minute);
		return false;
	}

	if (second < 0 || second > 59)
	{
		Logger::log_error("Invalid second: {}", second);
		return false;
	}

	return true;
}

// TTEFile

TTEFileWriter::TTEFileWriter(const std::wstring& file_path)
	: _file_path(file_path)
{
}

TTEFileWriter::~TTEFileWriter()
{
	if (_file.is_open())
	{
		_close();
	}
}

bool TTEFileWriter::add_event(const Date& date, const Event& event)
{
	if (!_file.is_open() && !_open())
	{
		return false;
	}

	char header[4];
	_file.read(header, 3);
	header[3] = '\0';

	if (std::string(header) != "TTE")
	{
		Logger::log_error("Invalid file header: {}", header);
		_close();
		return false;
	}

	encoded_date encoded_date;
	date.encode(encoded_date);

	encoded_event encoded_event;
	event.encode(encoded_event);

	const _DateBlock last_block = _last_date_block();

	if (last_block.date == date)
	{
		_file.seekg(last_block.start_offset + 2);

		uint32_t num_of_events;
		_file.read(reinterpret_cast<char*>(&num_of_events), sizeof(num_of_events));

		_file.seekp(last_block.start_offset + 2);

		num_of_events++;
		_file.write(reinterpret_cast<const char*>(&num_of_events), sizeof(num_of_events));

		_file.seekp(last_block.end_offset);

		_file.write(reinterpret_cast<const char*>(&encoded_event), sizeof(encoded_event));
	}
	else
	{
		uint16_t num_of_dates;
		_file.read(reinterpret_cast<char*>(&num_of_dates), sizeof(num_of_dates));

		_file.seekp(3);

		num_of_dates++;
		_file.write(reinterpret_cast<const char*>(&num_of_dates), sizeof(num_of_dates));

		_file.seekp(0, std::ios::end);

		_file.write(reinterpret_cast<const char*>(&encoded_date), sizeof(encoded_date));

		uint32_t num_of_events = 1;
		_file.write(reinterpret_cast<const char*>(&num_of_events), sizeof(num_of_events));

		_file.write(reinterpret_cast<const char*>(&encoded_event), sizeof(encoded_event));
	}

	_close();

	return true;
}

size_t TTEFileWriter::get_num_of_dates()
{
	if (!_file.is_open() && !_open())
	{
		return 0;
	}

	char header[4];
	_file.read(header, 3);
	header[3] = '\0';

	if (std::string(header) != "TTE")
	{
		Logger::log_error("Invalid file header: {}", header);
		_close();
		return 0;
	}

	uint16_t num_of_dates;
	_file.read(reinterpret_cast<char*>(&num_of_dates), sizeof(num_of_dates));

	_close();

	return num_of_dates;
}

bool TTEFileWriter::get_last_event(Event& event)
{
	size_t num_of_dates = get_num_of_dates();

	if (num_of_dates == 0)
	{
		return false;
	}

	_DateBlock last_block = _last_date_block();

	if (last_block.num_of_events == 0)
	{
		return false;
	}

	if (!_file.is_open() && !_open())
	{
		return false;
	}

	_file.seekg(last_block.end_offset - sizeof(encoded_event));

	encoded_event encoded_event;
	_file.read(reinterpret_cast<char*>(&encoded_event), sizeof(encoded_event));

	event = Event::decode(encoded_event);

	_close();

	return true;
}

bool TTEFileWriter::get_last_date(Date& date)
{
	size_t num_of_dates = get_num_of_dates();

	if (num_of_dates == 0)
	{
		return false;
	}

	if (!_file.is_open() && !_open())
	{
		return false;
	}

	_DateBlock last_block = _last_date_block();

	date = last_block.date;

	_close();

	return true;
}

bool TTEFileWriter::_open()
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
		Logger::log_error("Unable to open file: {}", (char*)_file_path.c_str());
		return false;
	}

	return true;
}

bool TTEFileWriter::_close()
{
	_file.close();
	if (_file.is_open())
	{
		Logger::log_error("Unable to close file: {}", (char*)_file_path.c_str());
	}

	return true;
}

bool TTEFileWriter::_create()
{
	if (!std::filesystem::exists(_file_path))
	{
		std::filesystem::create_directories(std::filesystem::path(_file_path).parent_path());
	}

	_file.open(_file_path, std::ios::out | std::ios::binary);

	if (!_file.is_open())
	{
		Logger::log_error("Unable to create file: {}", (char*)_file_path.c_str());
		return false;
	}

	_file.write("TTE", 3);

	uint16_t num_of_dates = 0;
	_file.write(reinterpret_cast<const char*>(&num_of_dates), sizeof(num_of_dates));

	_file.close();

	return true;
}

const TTEFileWriter::_DateBlock TTEFileWriter::_last_date_block()
{
	_DateBlock last_date_block;
	bool was_open = _file.is_open();

	if (!was_open && !_open())
	{
		return last_date_block;
	}

	size_t old_position = _file.tellg();

	_file.seekg(3);

	uint16_t num_of_dates;
	_file.read(reinterpret_cast<char*>(&num_of_dates), sizeof(num_of_dates));

	if (num_of_dates == 0)
	{
		goto CLEANUP;
	}

	for (int i = 0; i < (int)num_of_dates - 1; i++)
	{
		_file.seekg(2, std::ios::cur);

		uint32_t num_of_events;
		_file.read(reinterpret_cast<char*>(&num_of_events), sizeof(num_of_events));

		_file.seekg(num_of_events * sizeof(encoded_event), std::ios::cur);
	}

	last_date_block.start_offset = _file.tellg();

	encoded_date encoded_date;
	_file.read(reinterpret_cast<char*>(&encoded_date), sizeof(encoded_date));

	last_date_block.date = Date::decode(encoded_date);

	uint32_t num_of_events;
	_file.read(reinterpret_cast<char*>(&num_of_events), sizeof(num_of_events));

	last_date_block.num_of_events = num_of_events;

	_file.seekg(num_of_events * sizeof(encoded_event), std::ios::cur);

	last_date_block.end_offset = _file.tellg();

CLEANUP:
	if (!was_open)
	{
		_close();
	}

	_file.seekg(old_position);

	return last_date_block;
}