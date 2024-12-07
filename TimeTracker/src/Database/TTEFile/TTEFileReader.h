#pragma once

#include <fstream>
#include <filesystem>
#include <string>
#include <iterator>
#include <vector>
#include <functional>

#include <stdint.h>
#include <cstddef>
#include <cstring>

#include "TTEFileDate.h"
#include "TTEFileEvent.h"

#include "../../Utils/Logger.h"
#include "../../Utils/Filter.h"
#include "../../Utils/StringConverter.h"

class TTEFileReader
{
public:
	TTEFileReader(const std::wstring& file_path);
	~TTEFileReader();

private:
	using domain_id = uint8_t;
	using entity_id = uint16_t;

public:
	struct Date
	{
		Date();
		Date(TTEFileDate date);
		Date(uint8_t year, uint8_t month, uint8_t day);

		uint8_t year, month, day;

		bool operator==(const Date& other) const;
	};

private:
	struct _DateBlock;
public:
	class DateIterator
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = Date;
		using difference_type = std::ptrdiff_t;
		using pointer = Date*;
		using reference = Date&;

	public:
		Date operator*() const;

		DateIterator& operator++();
		DateIterator operator++(int);

		DateIterator& operator--();
		DateIterator operator--(int);

		bool operator==(const DateIterator& other) const;

	private:
		DateIterator(const std::vector<_DateBlock>::const_iterator& it);

		std::vector<struct _DateBlock>::const_iterator _it;

		friend class TTEFileReader;
	};

	struct DateRange
	{
	public:
		DateIterator begin() const;
		DateIterator end() const;
	private:
		DateRange(const DateIterator& begin, const DateIterator& end, uint16_t num_of_dates);

		DateIterator _begin;
		DateIterator _end;

		uint16_t _num_of_dates;

		friend class TTEFileReader;
	};

	using DateFilter = Filter<const Date&>;
	using DateWalker = std::function<bool(const Date&)>;

	DateRange dates();
	DateRange dates(DateFilter filter);

	bool date_exists(const Date& date);

	uint16_t count_dates();
	uint16_t count_dates(DateFilter filter);
	bool date_exists(DateFilter filter);

	void walk_dates(DateFilter filter, DateWalker function);

public:
	struct Event
	{
		Event();
		Event(Date date, TTEFileEvent event);
		Event(Date date, uint8_t hour, uint8_t minute, uint8_t second, entity_id entity);

		Date date;
		uint8_t hour, minute, second;
		entity_id entity;

		bool operator==(const Event& other) const;
	};

private:
	using _EventIndex = uint64_t;

	using _EventGetter = std::function<Event(_EventIndex)>;
	using _EventTester = std::function<bool(_EventIndex)>;

public:
	using EventFilter = Filter<const Event&>;
	using EventWalker = std::function<bool(const Event&)>;

private:
	struct _EventFilterProxy
	{
	public:
		Event get_event(_EventIndex index) const;
		bool test_event(_EventIndex index) const;

	private:
		_EventFilterProxy(EventFilter filter, TTEFileReader* reader);

		EventFilter _filter;
		TTEFileReader* _reader;

		friend class TTEFileReader;
	};

public:
	class EventIterator
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = Event;
		using difference_type = std::ptrdiff_t;
		using pointer = Event*;
		using reference = Event&;

	public:
		Event operator*() const;

		EventIterator& operator++();
		EventIterator operator++(int);

		EventIterator& operator--();
		EventIterator operator--(int);

		bool operator==(const EventIterator& other) const;

	private:
		EventIterator(_EventFilterProxy filter_proxy, _EventIndex index);

		_EventFilterProxy _filter_proxy;

		_EventIndex _index;

		friend class TTEFileReader;
	};

	struct EventRange
	{
	public:
		EventIterator begin() const;
		EventIterator end() const;

	private:
		EventRange(TTEFileReader* reader, _EventIndex start, _EventIndex stop, EventFilter filter);

		_EventIndex _start_index;
		_EventIndex _stop_index;

		_EventFilterProxy _filter_proxy;

		friend class TTEFileReader;
	};

public:
	EventRange events();
	EventRange events(EventFilter filter);

	bool event_exists(const Event& event);

	uint64_t count_events();
	uint64_t count_events(EventFilter filter);
	bool event_exists(EventFilter filter);

	void walk_events(EventFilter filter, EventWalker function);

private:
	std::wstring _file_path;
	std::fstream _file;

private:
	bool _open();
	bool _close();

private:
	bool _read_header();

private:
	struct _DateBlock
	{
		Date date = Date();
		uint32_t num_of_events = 0;

		uint64_t start_offset = 0;
	};

	uint16_t _num_of_dates = 0;
	std::vector<_DateBlock> _dates;

	bool _read_dates(DateFilter filter);

private:
	uint16_t _date_index(_EventIndex event) const;
	uint64_t _event_index(_EventIndex event) const;

	_EventIndex _event_location(uint16_t date_index, uint64_t event_index) const;

	uint64_t _event_offset(_EventIndex event) const;

	_EventIndex _event_count() const;

private:
	template <uint64_t N>
	struct _EncodedEventBuffer
	{
		TTEFileEvent::encoded_event events[N] = { 0 };

		uint64_t capacity = N;
		
		uint64_t start_index_in_buffer = 0;
		uint64_t stop_index_in_buffer = 0;

		_EventIndex start_index_in_file = 0;
		_EventIndex stop_index_in_file = 0;

		uint64_t size() const;

		bool full() const;
		bool empty() const;

		void clear();

		void shift_left(uint64_t n);
		void shift_right(uint64_t n, _EventIndex max_location);

		TTEFileEvent::encoded_event& operator[](_EventIndex event);
	};

	bool _populate_encoded_event_buffer();

	bool _shift_encoded_events_left(uint64_t n);
	bool _shift_encoded_events_right(uint64_t n);

	bool _move_encoded_events(_EventIndex start);

	bool _move_encoded_events_to_start();
	bool _move_encoded_events_to_end();
	
private:
	static constexpr uint64_t _encoded_event_buffer_forwards_capacity = 768;
	static constexpr uint64_t _encoded_event_buffer_backwards_capacity = 256;
	static constexpr uint64_t _encoded_event_buffer_capacity = _encoded_event_buffer_forwards_capacity + _encoded_event_buffer_backwards_capacity;

	_EncodedEventBuffer<_encoded_event_buffer_capacity> _encoded_event_buffer = { 0 };

	bool _get_event(_EventIndex index, Event& event);
};

template <uint64_t N>
uint64_t TTEFileReader::_EncodedEventBuffer<N>::size() const
{
	return (stop_index_in_buffer - start_index_in_buffer);
}

template <uint64_t N>
bool TTEFileReader::_EncodedEventBuffer<N>::full() const
{
	return size() == capacity - 1;
}

template <uint64_t N>
bool TTEFileReader::_EncodedEventBuffer<N>::empty() const
{
	return size() == 0;
}

template <uint64_t N>
void TTEFileReader::_EncodedEventBuffer<N>::clear()
{
	start_index_in_buffer = 0;
	stop_index_in_buffer = 0;
}

template <uint64_t N>
void TTEFileReader::_EncodedEventBuffer<N>::shift_left(uint64_t n)
{
	if (n >= size())
	{
		Logger::log_error("Cannot shift left by more than the size of the buffer");
		clear();
		return;
	}

	uint64_t new_first_element = min(start_index_in_buffer + n, capacity - 1);
	if (new_first_element < start_index_in_buffer)
		new_first_element = capacity - 1;
	uint64_t new_last_element = min(stop_index_in_buffer + n, capacity - 1);
	if (new_last_element < stop_index_in_buffer)
		new_last_element = capacity - 1;

	uint64_t new_size = new_last_element - new_first_element;

	std::memmove(reinterpret_cast<char*>(events + new_first_element), reinterpret_cast<char*>(events + start_index_in_buffer), new_size * sizeof(TTEFileEvent::encoded_event));

	start_index_in_buffer = new_first_element;
	stop_index_in_buffer = new_last_element;

	stop_index_in_file = start_index_in_file + new_size;
}

template <uint64_t N>
void TTEFileReader::_EncodedEventBuffer<N>::shift_right(uint64_t n, _EventIndex event_count)
{
	if (n >= size())
	{
		Logger::log_error("Cannot shift right by more than the size of the buffer");
		clear();
		return;
	}

	uint64_t new_first_element = max(start_index_in_buffer - n, 0);
	if (new_first_element > start_index_in_buffer)
		new_first_element = 0;
	uint64_t new_last_element = max(stop_index_in_buffer - n, 0);
	if (new_last_element > stop_index_in_buffer)
		new_last_element = 0;

	uint64_t new_size = new_last_element - new_first_element;

	std::memmove(reinterpret_cast<char*>(events + new_first_element), reinterpret_cast<char*>(events + stop_index_in_buffer - new_size), new_size * sizeof(TTEFileEvent::encoded_event));

	start_index_in_buffer = new_first_element;
	stop_index_in_buffer = new_last_element;

	start_index_in_file = stop_index_in_file - new_size;
}

template <uint64_t N>
TTEFileEvent::encoded_event& TTEFileReader::_EncodedEventBuffer<N>::operator[](_EventIndex event)
{
	if (event < start_index_in_file || event >= stop_index_in_file)
	{
		Logger::log_error("Event index out of bounds");
		throw std::out_of_range("Event index out of bounds");
	}

	uint64_t index = start_index_in_buffer + event - start_index_in_file;
	return events[index];
}