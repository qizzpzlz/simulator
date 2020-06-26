#include "../dependencies/json11.hpp"
#include <string>
#include <string_view>
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <cstring>
#include "file_reader.h"
#include "parameters.h"
#include "job.h"

namespace genetic
{
	std::vector<Entry> read_binary(const char* file_name)
	{
		auto file = std::ifstream(file_name, std::ios::binary | std::ios::ate);

		if (!file)
		{
			std::cerr << "Can't find a binary file for the job table. (" << file_name << ')' << std::endl;
			exit(1);
		}

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

		char* current = reinterpret_cast<char*>(buffer.data());
		char* end = current + size;
		std::vector<Entry> entries;
		entries.reserve(RESERVED_ENTRIES_COUNT);
		
		while (current < end)
			entries.push_back(Entry::from_binary(&current));

		entries.shrink_to_fit();

		if (CONSOLE_OUTPUT)
			std::cout << "Binary job table is successfully loaded." << std::endl;
		
		return entries;
	}

	void read_host_json(const char* file_name, const char* binary_output_path)
	{
		using namespace json11;
		std::ifstream file(file_name);
		if (!file)
		{
			std::cerr << "Can't find a host file. (" << file_name << ')' << std::endl;
			exit(1);
		}

		std::cout << "Parsing hosts json..." << std::endl;

		std::string buffer((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

		std::string err;
		const Json json = Json::parse(buffer, err);

		auto array = json.array_items();

		std::vector<HostInfo> hosts;
		hosts.reserve(11000);
		for (auto& item : array)
		{
			std::cout << "";

			auto objs = item.object_items();

			int max_slots = objs["MAX_SLOT"].int_value();
			unsigned char downcasted = static_cast<unsigned char>(max_slots);

			hosts.push_back(HostInfo{ static_cast<float>(objs["CPU_FACTOR"].number_value()), downcasted });
		}

		std::filebuf binary_output;
		binary_output.open(binary_output_path, std::ios::out | std::ios::binary);

		unsigned short count = hosts.size();
		binary_output.sputn(reinterpret_cast<const char*>(&count), sizeof(unsigned short));

		for (const auto& host : hosts)
		{
			host.write(binary_output);
		}
	}

	std::vector<HostInfo> read_host_binary()
	{
		const std::filesystem::path binary_path = "hosts.bin";
		if (!std::filesystem::exists(binary_path))
		{
			read_host_json(host_file_path, "hosts.bin");
		}

		auto file = std::ifstream(binary_path, std::ios::binary | std::ios::ate);
		if (!file)
		{
			std::cerr << "Can't find a binary host file. (" << binary_path << ')' << std::endl;
			exit(1);
		}

		file.seekg(0, std::ifstream::end);
		const size_t size = file.tellg();
		file.seekg(0, std::ifstream::beg);

		if (size == 0) return std::vector<HostInfo>{};

		std::vector<std::byte> buffer(size);

		if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
			throw std::runtime_error("");

		int read = 0;

		std::byte* ptr = buffer.data();

		unsigned short count = *reinterpret_cast<unsigned short*>(ptr);
		ptr += sizeof(unsigned short);

		HostInfo* current = reinterpret_cast<HostInfo*>(ptr);
		std::vector<HostInfo> hosts;
		hosts.reserve(11000);
		memcpy(hosts.data(), current, sizeof(current) / sizeof(Host));

		hosts.assign(current, current + count);

		if (CONSOLE_OUTPUT)
			std::cout << "Binary host list is successfully loaded." << std::endl;
		
		return hosts;
	}
}

