#pragma once

#include <fstream>
#include <filesystem>
#include <string>
#include <iterator>

#include <stdint.h>
#include <cstddef>

#include "../Utils/Logger.h"

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

class TTRFile
{
public:
	using domain_id = uint8_t;
	using entity_id = uint16_t;

public:
	TTRFile(const std::string& file_path);
	~TTRFile();

	bool add_domain(const std::string& domain);
	domain_id get_domain_id(const std::string& domain);
	bool domain_exists(const std::string& domain);

	bool add_entity(domain_id id, const std::string& entity);
	entity_id get_entity_id(domain_id id, const std::string& entity);
	bool entity_exists(domain_id id, const std::string& entity);

private:
	std::string _file_path;
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

private:
	template <typename EntryType>
	class TTREntryRange;

	template <typename EntryType>
	class TTREntryIterator
	{
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = const EntryType;

		using pointer = const value_type*;
		using reference = const value_type&;

		using iterator_category = std::forward_iterator_tag;

		TTREntryIterator();
		~TTREntryIterator();

		TTREntryIterator(TTREntryIterator& other);
		TTREntryIterator& operator=(TTREntryIterator& other);

		TTREntryIterator(const TTREntryIterator& other);
		TTREntryIterator& operator=(const TTREntryIterator& other);

		bool operator==(const TTREntryIterator& other) const;

	protected:
		friend class DomainRange;

		TTREntryIterator(TTRFile* file, uint32_t offset, uint16_t num_of_entries, uint8_t current_entry);

	protected:
		TTRFile* _ttr_file;

		uint32_t _offset_to_current_entry;
		uint16_t _num_of_entries;
		uint8_t _current_entry;

	protected:
		bool _file_ready = false;
		bool _file_was_open_before = false;

		bool _prepare_file();
	};

public:
	struct Domain
	{
		domain_id id;
		std::string name;
	};

	class DomainIterator;

	class DomainIterator : public TTREntryIterator<const Domain>
	{
	public:
		friend class DomainRange;

		using TTREntryIterator<const Domain>::TTREntryIterator;

		const Domain operator*() const;

		DomainIterator& operator++();
		DomainIterator operator++(int);
	};

	static_assert(std::forward_iterator<DomainIterator>);

	class DomainRange
	{
	public:
		DomainRange(TTRFile* file);

		DomainIterator begin();
		DomainIterator end();

	private:
		TTRFile* _file;
	};

	DomainRange	domains();
};


template <typename EntryType>
TTRFile::TTREntryIterator<EntryType>::TTREntryIterator()
	: _ttr_file(nullptr), _offset_to_current_entry(0), _num_of_entries(0), _current_entry(0)
{
}

template <typename EntryType>
TTRFile::TTREntryIterator<EntryType>::~TTREntryIterator()
{
	if (!_file_was_open_before)
	{
		_ttr_file->_close();
	}
}

template <typename EntryType>
TTRFile::TTREntryIterator<EntryType>::TTREntryIterator(TTREntryIterator& other)
	: _ttr_file(other._ttr_file), _offset_to_current_entry(other._offset_to_current_entry), _num_of_entries(other._num_of_entries), _current_entry(other._current_entry)
{
	_file_ready = _prepare_file();
}

template <typename EntryType>
TTRFile::TTREntryIterator<EntryType>& TTRFile::TTREntryIterator<EntryType>::operator=(TTREntryIterator& other)
{
	_ttr_file = other._ttr_file;
	_offset_to_current_entry = other._offset_to_current_entry;
	_num_of_entries = other._num_of_entries;
	_current_entry = other._current_entry;

	_file_ready = _prepare_file();

	return *this;
}

template <typename EntryType>
TTRFile::TTREntryIterator<EntryType>::TTREntryIterator(const TTREntryIterator& other)
	: _ttr_file(other._ttr_file), _offset_to_current_entry(other._offset_to_current_entry), _num_of_entries(other._num_of_entries), _current_entry(other._current_entry)
{
	_file_ready = _prepare_file();
}

template <typename EntryType>
TTRFile::TTREntryIterator<EntryType>& TTRFile::TTREntryIterator<EntryType>::operator=(const TTREntryIterator& other)
{
	_ttr_file = other._ttr_file;
	_offset_to_current_entry = other._offset_to_current_entry;
	_num_of_entries = other._num_of_entries;
	_current_entry = other._current_entry;

	_file_ready = _prepare_file();

	return *this;
}

template <typename EntryType>
bool TTRFile::TTREntryIterator<EntryType>::operator==(const TTREntryIterator& other) const
{
	return _ttr_file == other._ttr_file && _offset_to_current_entry == other._offset_to_current_entry && _num_of_entries == other._num_of_entries && _current_entry == other._current_entry;
}

template <typename EntryType>
TTRFile::TTREntryIterator<EntryType>::TTREntryIterator(TTRFile* file, uint32_t offset, uint16_t num_of_entries, uint8_t current_entry)
	: _ttr_file(file), _offset_to_current_entry(offset), _num_of_entries(num_of_entries), _current_entry(current_entry)
{
	_file_ready = _prepare_file();
}

template <typename EntryType>
bool TTRFile::TTREntryIterator<EntryType>::_prepare_file()
{
	_file_was_open_before = _ttr_file->_file.is_open();

	if (!_file_was_open_before && !_ttr_file->_open())
	{
		Logger::log_error("Failed to open file: {}", _ttr_file->_file_path);
		return false;
	}

	if (!_ttr_file->_read_info())
	{
		Logger::log_error("Failed to read file info");
		return false;
	}

	if (_current_entry >= _num_of_entries)
	{
		Logger::log_error("Invalid current entry: {}", _current_entry);
		return false;
	}

	return true;
}