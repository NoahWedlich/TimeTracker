#include "TTEFileDate.h"

TTEFileDate::TTEFileDate()
	: year(0), month(0), day(0)
{
}

TTEFileDate::TTEFileDate(uint8_t year, uint8_t month, uint8_t day)
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

void TTEFileDate::encode(encoded_date& date) const
{
	if (!check_validity())
	{
		Logger::append_info("Unable to encode date: {}-{}-{}", year, month, day);

		date = 0;
		return;
	}

	date = (uint16_t(year) << 9) | (uint16_t(month) << 5) | uint16_t(day);
}

const TTEFileDate TTEFileDate::decode(const encoded_date& date)
{
	TTEFileDate decoded_date;

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

bool TTEFileDate::operator==(const TTEFileDate& other) const
{
	return year == other.year && month == other.month && day == other.day;
}

bool TTEFileDate::check_validity() const
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