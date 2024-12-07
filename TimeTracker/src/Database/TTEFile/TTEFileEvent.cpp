#include "TTEFileEvent.h"

TTEFileEvent::TTEFileEvent()
	: entity(0), hour(0), minute(0), second(0)
{
}

TTEFileEvent::TTEFileEvent(entity_id entity, uint8_t hour, uint8_t minute, uint8_t second)
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

void TTEFileEvent::encode(encoded_event& event) const
{
	if (!check_validity())
	{
		Logger::append_info("Unable to encode event: {} {}:{}:{}", entity, hour, minute, second);

		event = 0;
		return;
	}

	event = (uint32_t(entity) << 17) | (uint32_t(hour) << 12) | (uint32_t(minute) << 6) | uint32_t(second);
}

const TTEFileEvent TTEFileEvent::decode(const encoded_event& event)
{
	TTEFileEvent decoded_event;

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

bool TTEFileEvent::check_validity() const
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