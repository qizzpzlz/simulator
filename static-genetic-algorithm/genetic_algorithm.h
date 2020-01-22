#pragma once
#include "file_reader.h"
#include "host.h"

namespace genetic
{
	constexpr char binary_file_path[] = "../static-genetic-algorithm/job-eligibility.small.bin";
	constexpr std::size_t NUM_INITIAL_POPULATION = 1000;
	
	inline static const std::vector<Entry> job_table = read_binary(binary_file_path);
	inline static std::vector<HostInfo> host_table = read_host_binary();
	inline static const std::vector<Host> host_prototypes = []
	{
		std::vector<Host> result;
		result.reserve(host_table.size());
		for (auto& info : host_table)
		{
			result.push_back(Host{ info.max_slots });
		}
		
		return result;
	}();
	inline static const unsigned short LENGTH = job_table.size();
	inline static const std::size_t NUM_HOSTS = host_table.size();

	void generate_initial_population();
}
