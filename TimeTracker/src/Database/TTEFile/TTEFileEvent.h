#pragma once

#include <stdint.h>

#include "../../Utils/Logger.h"

struct TTEFileEvent
{
	using entity_id = uint16_t;

	using encoded_event = uint32_t;

	TTEFileEvent();
	TTEFileEvent(entity_id entity, uint8_t hour, uint8_t minute, uint8_t second);

	entity_id entity;

	uint8_t hour;
	uint8_t minute;
	uint8_t second;

	void encode(encoded_event& event) const;
	static const TTEFileEvent decode(const encoded_event& event);

	bool check_validity() const;
};