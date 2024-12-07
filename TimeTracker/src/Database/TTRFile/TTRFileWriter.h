#pragma once

#include <fstream>
#include <filesystem>
#include <string>
#include <iterator>

#include <stdint.h>
#include <cstddef>

#include "../../Utils/Logger.h"

/*
* Time Tracker Registry File
* 
* Layout:
* 
* Start:
*   - 'TTR'                            3
*   - offset_to_domains_start          4
*   - offset_to_domains_end            4
*   - offset_to_entities               4
* 						               
* Domains:				               
*   - num_of_domains                   1
*   - {len: 1, name: len}              [num_of_domains]
* 						               
* Entities:				               
*  - num_of_entities                   2
*  - {domain_id: 1, len: 1, name: len} [num_of_entities]
*/

constexpr int DOMAINS_MIN_BLOCK_SIZE = 1024;

class TTRFileWriter
{
public:
	using domain_id = uint8_t;
	using entity_id = uint16_t;

public:
	TTRFileWriter(const std::wstring& file_path);
	~TTRFileWriter();

	bool add_domain(const std::string& domain);
	domain_id get_domain_id(const std::string& domain);
	bool domain_exists(const std::string& domain);

	bool add_entity(domain_id id, const std::string& entity);
	entity_id get_entity_id(domain_id id, const std::string& entity);
	bool entity_exists(domain_id id, const std::string& entity);

private:
	std::wstring _file_path;
	std::fstream _file;

private:
	bool _open();
	bool _close();

	bool _create();

private:
	uint32_t _offset_to_current_domain = 0;
	uint32_t _offset_to_domains_end = 0;
	uint8_t _num_of_domains = 0;


	uint32_t _offset_to_entities = 0;
	uint16_t _num_of_entities = 0;

private:
	bool _read_info();

	bool _update_header();

protected:
	uint32_t _offset_to_current_entry;
	uint16_t _num_of_entries;
	uint8_t _current_entry;
};
