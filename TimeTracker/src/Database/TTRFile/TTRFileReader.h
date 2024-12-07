#pragma once

#include <fstream>
#include <filesystem>
#include <string>
#include <iterator>
#include <vector>
#include <functional>

#include <stdint.h>
#include <cstddef>

#include "../../Utils/Logger.h"
#include "../../Utils/Filter.h"
#include "../../Utils/StringConverter.h"

class TTRFileReader
{
public:
	TTRFileReader(const std::wstring& file_path);
	~TTRFileReader();

public:
	using domain_id = uint8_t;
	using entity_id = uint16_t;

public:
	using DomainIterator = std::vector<std::string>::const_iterator;

	struct DomainRange
	{
	public:
		DomainIterator begin() const;
		DomainIterator end() const;

		size_t size() const;
	private:
		DomainRange(DomainIterator begin, DomainIterator end, uint8_t num_of_domains);

		DomainIterator _begin;
		DomainIterator _end;

		uint8_t _num_of_domains;

		friend class TTRFileReader;
	};

	using DomainFilter = Filter<domain_id, const std::string&>;
	using DomainWalker = std::function<bool(domain_id, const std::string&)>;

	DomainRange domains();
	DomainRange domains(DomainFilter filter);

	bool domain_exists(const std::string& domain);

	domain_id get_domain_id(const std::string& domain);

	uint8_t count_domains(DomainFilter filter);
	bool domain_exists(DomainFilter filter);

	void walk_domains(DomainFilter filter, DomainWalker function);

public:
	struct Entity
	{
		domain_id domain_id;
		std::string name;
	};

	using EntityIterator = std::vector<Entity>::const_iterator;

	struct EntityRange
	{
	public:
		EntityIterator begin() const;
		EntityIterator end() const;

		size_t size() const;
	private:
		EntityRange(EntityIterator begin, EntityIterator end, uint16_t num_of_entities);

		EntityIterator _begin;
		EntityIterator _end;

		uint16_t _num_of_entities;

		friend class TTRFileReader;
	};

	using EntityFilter = Filter<entity_id, const Entity&>;
	using EntityWalker = std::function<bool(entity_id, const Entity&)>;

	EntityRange entities();
	EntityRange entities(EntityFilter filter);

	bool entity_exists(const std::string& entity);
	bool entity_exists(domain_id id, const std::string& entity);

	entity_id get_entity_id(domain_id id, const std::string& entity);
	entity_id get_entity_id(const std::string& entity, domain_id* id = nullptr);

	std::string get_entity(entity_id id);
	std::string get_domain(entity_id id);

	uint16_t count_entities(EntityFilter filter);
	bool entity_exists(EntityFilter filter);

	void walk_entities(EntityFilter filter, EntityWalker function);

private:
	std::wstring _file_path;
	std::fstream _file;

private:
	bool _open();
	bool _close();

private:
	struct Header
	{
		uint32_t offset_to_domains_start;
		uint32_t offset_to_domains_end;
		uint32_t offset_to_entities;
	} _header;

	bool _read_header();

private:
	uint8_t _num_of_domains;
	std::vector<std::string> _domains;

	bool _read_domains(DomainFilter filter);

private:
	uint16_t _num_of_entities;
	std::vector<Entity> _entities;

	bool _read_entities(EntityFilter filter);
};