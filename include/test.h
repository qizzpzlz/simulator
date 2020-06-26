#pragma once
#include <filesystem>
#include <iostream>
#include <fstream>

using std::filesystem::path;

void verify_allocation_binaries(const path& old_bin, const path& generated_bin)
{
#pragma pack(push, 1)
	struct _Allocation_record_entry
	{
		uint64_t job_id;
		uint16_t host_id;
		int64_t start_time;
	};
#pragma pack(pop)

	std::ifstream file(old_bin, std::ios::binary | std::ios::ate);
	file.seekg(0, std::ifstream::end);
	std::size_t size = file.tellg();
	file.seekg(0, std::ifstream::beg);
	auto buffer = std::make_unique<std::byte[]>(size);
	file.read(reinterpret_cast<char*>(buffer.get()), size);

	std::size_t length1 = size / sizeof(uint16_t);
	std::vector<uint16_t> old_result(size);
	memcpy(old_result.data(), reinterpret_cast<const uint16_t*>(buffer.get()), size);

	file.close();

	file = std::ifstream(generated_bin, std::ios::binary | std::ios::ate);
	file.seekg(0, std::ifstream::end);
	size = file.tellg();
	file.seekg(0, std::ifstream::beg);
	buffer = std::make_unique<std::byte[]>(size);
	file.read(reinterpret_cast<char*>(buffer.get()), size);

	std::size_t length2 = size / sizeof(_Allocation_record_entry);

	if (length1 != length2)
		throw std::runtime_error("");
	
	std::vector<_Allocation_record_entry> gen_result(size);
	memcpy(gen_result.data(), reinterpret_cast<const _Allocation_record_entry*>(buffer.get()), size);

	for (auto i = 0; i < length2; ++i)
	{
		if (old_result[i] != gen_result[i].host_id)
			throw std::runtime_error("");
	}

	std::cout << "Test passed." << std::endl;

}