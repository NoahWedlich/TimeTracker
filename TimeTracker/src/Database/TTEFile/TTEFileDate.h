#pragma once

#include <stdint.h>

#include "../../Utils/Logger.h"

struct TTEFileDate
{
	using encoded_date = uint16_t;

	TTEFileDate();
	TTEFileDate(uint8_t year, uint8_t month, uint8_t day);

	uint8_t year, month, day;

	void encode(encoded_date& date) const;
	static const TTEFileDate decode(const encoded_date& date);

	bool operator==(const TTEFileDate& other) const;

	bool check_validity() const;
};