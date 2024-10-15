#pragma once

#include <fstream>
#include <filesystem>
#include <string>

#include <stdint.h>
#include <time.h>

#include "../Utils/Logger.h"

/*
* Time Tracker Event File
* 
* Layout:
*
* Start:
*   - 'TTE'            3
*   - num_of_dates     2
*   - {
*       Date:          2
*       num_of_events: 4
*       { Event: 4 }   [num_of_events]
*     }                [num_of_dates]
*/

class TTEFile
{
public:
	using encoded_date = uint16_t;

	struct Date
	{
		Date();
		Date(uint8_t year, uint8_t month, uint8_t day);

		uint8_t year;
		uint8_t month;
		uint8_t day;

		void encode(encoded_date& date) const;
		static const Date decode(const encoded_date& date);

		bool operator==(const Date& other) const;

		bool check_validity() const;
	};

	using entity_id = uint16_t;

	using encoded_event = uint32_t;

	struct Event
	{
		Event();
		Event(entity_id entity, uint8_t hour, uint8_t minute, uint8_t second);

		entity_id entity;

		uint8_t hour;
		uint8_t minute;
		uint8_t second;

		void encode(encoded_event& event) const;
		static const Event decode(const encoded_event& event);

		bool check_validity() const;
	};

public:
	TTEFile(const std::string& file_path);
	~TTEFile();

	bool add_event(const Date& date, const Event& event);

	size_t get_num_of_dates();

	bool get_last_date(Date& date);
	bool get_last_event(Event& event);

private:
	std::string _file_path;
	std::fstream _file;

private:
	bool _open();
	bool _close();

	bool _create();

private:
	struct _DateBlock
	{
		Date date;
		uint32_t num_of_events = 0;

		uint64_t start_offset = 0;
		uint64_t end_offset = 0;
	};

private:
	const _DateBlock _last_date_block();
};