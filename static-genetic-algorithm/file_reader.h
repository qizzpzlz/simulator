#pragma once
#include "../dependencies/json11.hpp"
#include <string>
#include <string_view>
#include <fstream>
#include <iostream>
#include <vector>

#pragma pack(push, 1)
struct UInt32_padded
{
	uint32_t value;
	std::byte padding;

	operator uint32_t() const { return value; }
	friend std::ostream& operator << (std::ostream& out, const UInt32_padded& value);
};
struct UInt16_padded
{
	uint16_t value;
	std::byte padding;

	operator uint16_t() const { return value; }
};
struct Str_20
{
	UInt32_padded a;
	UInt32_padded b;
	UInt32_padded c;
	UInt32_padded d;
};
#pragma pack(pop)

struct Entry
{
	Str_20 values;
	std::vector<UInt16_padded> hosts;

	[[nodiscard]] uint32_t submit_time() const { return values.a; }
	[[nodiscard]] uint32_t slots() const { return values.b; }
	[[nodiscard]] uint32_t cpu_time() const { return values.c; }
	[[nodiscard]] uint32_t non_cpu_time() const { return values.d; }
};



inline std::ostream& operator << (std::ostream& out, const UInt32_padded& value)
{
	out << value.value;
	return out;
}


inline std::vector<Entry> read_binary(const char* file_name)
{
	auto file = std::ifstream(file_name, std::ios::binary | std::ios::ate);

	if (!file)
		throw std::runtime_error("");

	//auto end = file.tellg();
	//file.seekg(0, std::ios::beg);

	//auto size = std::size_t(end - file.tellg());

	file.seekg(0, std::ifstream::end);
	const size_t size = file.tellg();
	file.seekg(0, std::ifstream::beg);

	if (size == 0) return std::vector<Entry>{};

	std::vector<std::byte> buffer(size);

	if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
		throw std::runtime_error("");

	int read = 0;
	char* current = reinterpret_cast<char*>(buffer.data());
	std::vector<Entry> entries;
	entries.reserve(20000);
	while (read < size)
	{		
		Str_20* front_ptr = reinterpret_cast<Str_20*>(current);
		current += 20;

		Entry entry;
		entry.values = *front_ptr;

		UInt16_padded array_size = *reinterpret_cast<UInt16_padded*>(current);
		current += 3;

		const std::size_t array_length_in_bytes = sizeof(UInt16_padded) * array_size.value;
		
		entry.hosts = std::vector<UInt16_padded>(array_size.value);
		UInt16_padded* hosts_ptr = entry.hosts.data();
		memcpy(hosts_ptr, current, array_length_in_bytes);
		current += array_length_in_bytes;

		entries.push_back(std::move(entry));

		read += 23 + array_length_in_bytes;
	}
	
	return entries;
}