#include "TTEFileReader.h"

TTEFileReader::TTEFileReader(const std::wstring& file_path)
	: _file_path(file_path), _file(), _num_of_dates(0), _dates()
{
}

TTEFileReader::~TTEFileReader()
{
	if (_file.is_open())
	{
		_file.close();
	}
}

TTEFileReader::Date::Date()
	: year(0), month(0), day(0)
{
}

TTEFileReader::Date::Date(TTEFileDate date)
	: year(date.year), month(date.month), day(date.day)
{
}

TTEFileReader::Date::Date(uint8_t year, uint8_t month, uint8_t day)
	: year(year), month(month), day(day)
{
}

bool TTEFileReader::Date::operator==(const Date& other) const
{
	return year == other.year && month == other.month && day == other.day;
}

TTEFileReader::Date TTEFileReader::DateIterator::operator*() const
{
	const _DateBlock block = *_it;
	return Date(block.date.year, block.date.month, block.date.day);
}

TTEFileReader::DateIterator& TTEFileReader::DateIterator::operator++()
{
	++_it;
	return *this;
}

TTEFileReader::DateIterator TTEFileReader::DateIterator::operator++(int)
{
	DateIterator it = *this;
	++_it;
	return it;
}

TTEFileReader::DateIterator& TTEFileReader::DateIterator::operator--()
{
	--_it;
	return *this;
}

TTEFileReader::DateIterator TTEFileReader::DateIterator::operator--(int)
{
	DateIterator it = *this;
	--_it;
	return it;
}

bool TTEFileReader::DateIterator::operator==(const DateIterator& other) const
{
	return _it == other._it;
}

TTEFileReader::DateIterator::DateIterator(const std::vector<_DateBlock>::const_iterator& it)
	: _it(it)
{
}

TTEFileReader::DateRange::DateRange(const DateIterator& begin, const DateIterator& end, uint16_t num_of_dates)
	: _begin(begin), _end(end), _num_of_dates(num_of_dates)
{
}

TTEFileReader::DateIterator TTEFileReader::DateRange::begin() const
{
	return _begin;
}

TTEFileReader::DateIterator TTEFileReader::DateRange::end() const
{
	return _end;
}

TTEFileReader::DateRange TTEFileReader::dates()
{
	return dates(DateFilter::empty());
}

TTEFileReader::DateRange TTEFileReader::dates(DateFilter filter)
{
	if (!_read_dates(filter))
	{
		Logger::append_info("Failed to read dates", StringConverter::to_utf8(_file_path));
	}

	return DateRange(_dates.begin(), _dates.end(), _num_of_dates);
}

bool TTEFileReader::date_exists(const Date& date)
{
	return date_exists(DateFilter::create([&date](const Date& d) {
		return d == date;
	}));
}

uint16_t TTEFileReader::count_dates()
{
	return count_dates(DateFilter::empty());
}

uint16_t TTEFileReader::count_dates(DateFilter filter)
{
	uint16_t count = 0;

	walk_dates(
		filter,
		[&count](const Date&)
		{
			++count;
			return true;
		}
	);

	return count;
}

bool TTEFileReader::date_exists(DateFilter filter)
{
	return count_dates(filter) > 0;
}

void TTEFileReader::walk_dates(DateFilter filter, DateWalker function)
{
	if (!_read_dates(filter))
	{
		Logger::append_info("Failed to walk dates");
		return;
	}

	for (const _DateBlock& block : _dates)
	{
		if (!function(Date(block.date.year, block.date.month, block.date.day)))
		{
			break;
		}
	}
}

TTEFileReader::Event::Event()
	: hour(0), minute(0), second(0), entity(0)
{
}

TTEFileReader::Event::Event(Date date, TTEFileEvent event)
	: date(date), hour(event.hour), minute(event.minute), second(event.second), entity(event.entity)
{
}

TTEFileReader::Event::Event(Date date, uint8_t hour, uint8_t minute, uint8_t second, entity_id entity)
	: date(date), hour(hour), minute(minute), second(second), entity(entity)
{
}

bool TTEFileReader::Event::operator==(const Event& other) const
{
	return hour == other.hour && minute == other.minute && second == other.second && entity == other.entity;
}

TTEFileReader::Event TTEFileReader::_EventFilterProxy::get_event(_EventIndex index) const
{
	if (!test_event(index))
	{
		Logger::log_error("Event does not satisfy filter");
		throw std::out_of_range("Event does not satisfy filter");
	}

	Event event;
	bool success = _reader->_get_event(index, event);

	if (!success)
	{
		Logger::log_error("Failed to get event");
		throw std::runtime_error("Failed to get event");
	}

	return event;
}

bool TTEFileReader::_EventFilterProxy::test_event(_EventIndex index) const
{
	if (index > _reader->_event_count())
	{
		return false;
	}

	if (index == _reader->_event_count())
	{
		return true;
	}

	Event event;
	bool success = _reader->_get_event(index, event);

	if (!success)
	{
		return false;
	}

	return _filter(event);
}

TTEFileReader::_EventFilterProxy::_EventFilterProxy(EventFilter filter, TTEFileReader* reader)
	: _filter(filter), _reader(reader)
{
}

TTEFileReader::Event TTEFileReader::EventIterator::operator*() const
{
	return _filter_proxy.get_event(_index);
}

TTEFileReader::EventIterator& TTEFileReader::EventIterator::operator++()
{
	while (!_filter_proxy.test_event(++_index))
	{
	}
	return *this;
}

TTEFileReader::EventIterator TTEFileReader::EventIterator::operator++(int)
{
	EventIterator it = *this;
	while (!_filter_proxy.test_event(++_index))
	{
	}
	return it;
}

TTEFileReader::EventIterator& TTEFileReader::EventIterator::operator--()
{
	while (!_filter_proxy.test_event(--_index))
	{
	}
	return *this;
}

TTEFileReader::EventIterator TTEFileReader::EventIterator::operator--(int)
{
	EventIterator it = *this;
	while (!_filter_proxy.test_event(--_index))
	{
	}
	return it;
}

bool TTEFileReader::EventIterator::operator==(const EventIterator& other) const
{
	return _index == other._index;
}

TTEFileReader::EventIterator::EventIterator(_EventFilterProxy filter_proxy, _EventIndex index)
	: _filter_proxy(filter_proxy), _index(index)
{
}

TTEFileReader::EventIterator TTEFileReader::EventRange::begin() const
{
	return EventIterator(_filter_proxy, _start_index);
}

TTEFileReader::EventIterator TTEFileReader::EventRange::end() const
{
	return EventIterator(_filter_proxy, _stop_index);
}

TTEFileReader::EventRange::EventRange(TTEFileReader* reader, _EventIndex start, _EventIndex stop, EventFilter filter)
	: _start_index(start), _stop_index(stop), _filter_proxy(filter, reader)
{
	while (!_filter_proxy.test_event(_start_index) && _start_index < _stop_index)
	{
		++_start_index;
	}
}

TTEFileReader::EventRange TTEFileReader::events()
{
	return events(EventFilter::empty());
}

TTEFileReader::EventRange TTEFileReader::events(EventFilter filter)
{
	_read_dates(DateFilter::empty());

	return EventRange(this, 0, _event_count(), filter);
}

bool TTEFileReader::event_exists(const Event& event)
{
	return event_exists(EventFilter::create([&event](const Event& e) {
		return e == event;
	}));
}

uint64_t TTEFileReader::count_events()
{
	return count_events(EventFilter::empty());
}

uint64_t TTEFileReader::count_events(EventFilter filter)
{
	uint64_t count = 0;

	walk_events(
		filter,
		[&count](const Event&)
		{
			++count;
			return true;
		}
	);

	return count;
}

bool TTEFileReader::event_exists(EventFilter filter)
{
	return count_events(filter) > 0;
}

void TTEFileReader::walk_events(EventFilter filter, EventWalker function)
{
	for (Event event : events(filter))
	{
		if (!function(event))
		{
			break;
		}
	}
}

bool TTEFileReader::_open()
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

bool TTEFileReader::_close()
{
	_file.close();

	if (_file.is_open())
	{
		Logger::log_error("Failed to close file: {}", StringConverter::to_utf8(_file_path));
		return false;
	}

	return true;
}

bool TTEFileReader::_read_header()
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

	if (std::strcmp(magic, "TTE") != 0)
	{
		Logger::log_error("Invalid file format: {}", StringConverter::to_utf8(_file_path));

		_close();

		return false;
	}

	_file.read(reinterpret_cast<char*>(&_num_of_dates), sizeof(_num_of_dates));

	return _close();
}

bool TTEFileReader::_read_dates(DateFilter filter)
{
	if (!_read_header())
	{
		Logger::append_info("Failed to read dates");
		return false;
	}

	if (!_open())
	{
		Logger::append_info("Failed to read dates");
		return false;
	}

	_file.seekg(5, std::ios::beg);

	_dates.clear();
	_dates.reserve(_num_of_dates);

	for (uint16_t i = 0; i < _num_of_dates; ++i)
	{
		_DateBlock block;

		TTEFileDate::encoded_date encoded_date = 0;
		_file.read(reinterpret_cast<char*>(&encoded_date), sizeof(encoded_date));

		TTEFileDate raw_date = TTEFileDate::decode(encoded_date);

		uint32_t num_of_events = 0;
		_file.read(reinterpret_cast<char*>(&num_of_events), sizeof(num_of_events));

		Date date(raw_date);

		block.date = date;
		block.num_of_events = num_of_events;
		block.start_offset = _file.tellg();

		if (filter(date))
			_dates.push_back(block);

		size_t offset = num_of_events * sizeof(TTEFileEvent::encoded_event);
		_file.seekg(offset, std::ios::cur);
	}

	return _close();
}

uint16_t TTEFileReader::_date_index(_EventIndex event) const
{
	uint16_t date_index = 0;

	uint64_t offset = 0;

	for (const _DateBlock& block : _dates)
	{
		offset += block.num_of_events;
		if (event < offset)
		{
			return date_index;
		}
		++date_index;
	}

	return 0;
}

uint64_t TTEFileReader::_event_index(_EventIndex event) const
{
	uint64_t offset = 0;

	for (const _DateBlock& block : _dates)
	{
		if (event < offset + block.num_of_events)
		{
			return event - offset;
		}
		offset += block.num_of_events;
	}

	return 0;
}

TTEFileReader::_EventIndex TTEFileReader::_event_location(uint16_t date_index, uint64_t event_index) const
{
	if (date_index >= _dates.size() || event_index >= _dates[date_index].num_of_events)
	{
		return _event_count();
	}

	uint64_t offset = 0;

	for (uint16_t i = 0; i < date_index; ++i)
	{
		offset += _dates[i].num_of_events;
	}

	return offset + event_index;
}

uint64_t TTEFileReader::_event_offset(_EventIndex event) const
{
	uint64_t date_offset = _dates[_date_index(event)].start_offset;
	uint64_t event_offset = _event_index(event) * sizeof(TTEFileEvent::encoded_event);

	return date_offset + event_offset;
}

TTEFileReader::_EventIndex TTEFileReader::_event_count() const
{
	uint64_t offset = 0;

	for (const _DateBlock& block : _dates)
	{
		offset += block.num_of_events;
	}

	return offset;
}

bool TTEFileReader::_populate_encoded_event_buffer()
{
	if (_encoded_event_buffer.full())
	{
		return true;
	}

	if (!_open())
	{
		Logger::append_info("Failed to fill event buffer");
		return false;
	}

	if (_num_of_dates == 0 && !_read_dates(DateFilter::empty()))
	{
		Logger::append_info("Failed to fill event buffer");
		return false;
	}

	uint64_t events_to_read_front = min(_encoded_event_buffer.start_index_in_buffer, _encoded_event_buffer.start_index_in_file);

	_EventIndex current_location = _encoded_event_buffer.start_index_in_file - events_to_read_front;
	uint64_t events_read_front = 0;

	while (events_read_front < events_to_read_front)
	{
		uint16_t date_index = _date_index(current_location);
		uint64_t event_index = _event_index(current_location);

		uint64_t events_to_read = min(events_to_read_front - events_read_front, _dates[date_index].num_of_events - event_index);

		uint64_t offset = _event_offset(current_location);

		_file.seekg(offset, std::ios::beg);

		_file.read(reinterpret_cast<char*>(_encoded_event_buffer.events + events_read_front), events_to_read * sizeof(TTEFileEvent::encoded_event));

		events_read_front += events_to_read;
		current_location += events_to_read;
	}

	uint64_t events_to_shift = _encoded_event_buffer.start_index_in_buffer - events_to_read_front;

	if (events_to_shift > 0)
	{
		TTEFileDate::encoded_date* buffer = new TTEFileDate::encoded_date[_encoded_event_buffer.size()];
		std::memcpy(buffer, _encoded_event_buffer.events + _encoded_event_buffer.start_index_in_buffer, _encoded_event_buffer.size() * sizeof(TTEFileDate::encoded_date));

		std::memcpy(_encoded_event_buffer.events + _encoded_event_buffer.start_index_in_buffer - events_to_shift, buffer, _encoded_event_buffer.size() * sizeof(TTEFileDate::encoded_date));
		delete[] buffer;
	}

	_encoded_event_buffer.start_index_in_buffer = 0;
	_encoded_event_buffer.stop_index_in_buffer -= events_to_shift;

	_encoded_event_buffer.start_index_in_file -= events_to_read_front;

	uint64_t events_to_read_back = min(_encoded_event_buffer.capacity - _encoded_event_buffer.stop_index_in_buffer, _event_count() - _encoded_event_buffer.stop_index_in_file);

	current_location = _encoded_event_buffer.stop_index_in_file;
	uint64_t events_read_back = 0;

	while (events_read_back < events_to_read_back)
	{
		uint16_t date_index = _date_index(current_location);
		uint64_t event_index = _event_index(current_location);

		uint64_t events_to_read = min(events_to_read_back - events_read_back, _dates[date_index].num_of_events - event_index);

		uint64_t offset = _event_offset(current_location);

		_file.seekg(offset, std::ios::beg);

		_file.read(reinterpret_cast<char*>(_encoded_event_buffer.events + _encoded_event_buffer.stop_index_in_buffer + events_read_back), events_to_read * sizeof(TTEFileEvent::encoded_event));

		events_read_back += events_to_read;
		current_location += events_to_read;
	}

	_encoded_event_buffer.stop_index_in_buffer += events_read_back;

	_encoded_event_buffer.stop_index_in_file += events_read_back;

	return _close();
}

bool TTEFileReader::_shift_encoded_events_left(uint64_t n)
{
	if (n >= _encoded_event_buffer.start_index_in_file)
	{
		Logger::log_warning("Cannot shift events left by {}", n);
		Logger::append_info("Current start: {}", _encoded_event_buffer.start_index_in_file);
		return _move_encoded_events_to_start();
	}

	_encoded_event_buffer.shift_left(n);
	return _populate_encoded_event_buffer();
}

bool TTEFileReader::_shift_encoded_events_right(uint64_t n)
{
	if (_encoded_event_buffer.stop_index_in_file + n > _event_count())
	{
		Logger::log_warning("Cannot shift events right by {}", n);
		Logger::append_info("Current end: {}, Last event: {}", _encoded_event_buffer.stop_index_in_file, _event_count() - 1);
		return _move_encoded_events_to_end();
	}

	_encoded_event_buffer.shift_right(n, _event_count());
	return _populate_encoded_event_buffer();
}

bool TTEFileReader::_move_encoded_events(_EventIndex location)
{
	if (location >= _encoded_event_buffer.start_index_in_file && location <= _encoded_event_buffer.stop_index_in_file)
	{
		return _populate_encoded_event_buffer();
	}
	else if (location < _encoded_event_buffer.start_index_in_file && _encoded_event_buffer.start_index_in_file - location < _encoded_event_buffer.size())
	{
		return _shift_encoded_events_left(_encoded_event_buffer.start_index_in_file - location);
	}
	else if (location > _encoded_event_buffer.stop_index_in_file && location - _encoded_event_buffer.stop_index_in_file < _encoded_event_buffer.size())
	{
		return _shift_encoded_events_right(location - _encoded_event_buffer.stop_index_in_file);
	}
	else
	{
		_encoded_event_buffer.clear();
		_encoded_event_buffer.start_index_in_file = location;
		_encoded_event_buffer.stop_index_in_file = location;
	}

	return _populate_encoded_event_buffer();
}

bool TTEFileReader::_move_encoded_events_to_start()
{
	return _move_encoded_events(0);
}

bool TTEFileReader::_move_encoded_events_to_end()
{
	return _move_encoded_events(_event_count() - 1);
}

bool TTEFileReader::_get_event(_EventIndex event_index, Event& event)
{

	if (event_index >= _event_count())
	{
		Logger::log_error("Event index out of bounds: {}", event_index);
		return false;
	}

	if (event_index >= _encoded_event_buffer.start_index_in_file && event_index < _encoded_event_buffer.stop_index_in_file)
	{
		uint16_t date_index = _date_index(event_index);

		TTEFileEvent::encoded_event encoded_event = _encoded_event_buffer[event_index];
		TTEFileEvent raw_event = TTEFileEvent::decode(encoded_event);

		event = Event(_dates[date_index].date, raw_event);

		return true;
	}

	uint64_t distance_to_start = _encoded_event_buffer.start_index_in_file - event_index;
	uint64_t distance_to_end = event_index - _encoded_event_buffer.stop_index_in_file;

	uint64_t events_to_shift = min(_encoded_event_buffer_forwards_capacity, _encoded_event_buffer.size());

	if (event_index < _encoded_event_buffer.start_index_in_file && distance_to_start < events_to_shift)
	{
		events_to_shift = min(events_to_shift, _encoded_event_buffer.start_index_in_file);
		if (!_shift_encoded_events_left(events_to_shift))
		{
			Logger::append_info("Failed to get event");
			return false;
		}
		return _get_event(event_index, event);
	}
	else if (event_index >= _encoded_event_buffer.stop_index_in_file && distance_to_end < events_to_shift)
	{
		events_to_shift = min(events_to_shift, _event_count() - _encoded_event_buffer.stop_index_in_file);
		if (!_shift_encoded_events_right(events_to_shift))
		{
			Logger::append_info("Failed to get event");
			return false;
		}
		return _get_event(event_index, event);
	}
	else
	{
		if (!_move_encoded_events(event_index))
		{
			Logger::append_info("Failed to get event");
			return false;
		}
		return _get_event(event_index, event);
	}
}