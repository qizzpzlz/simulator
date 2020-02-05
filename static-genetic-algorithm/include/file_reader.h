#pragma once
#include <string>
#include <fstream>
#include <vector>
#include "host.h"

namespace genetic
{
	constexpr char binary_file_path[] = "../static-genetic-algorithm/job-eligibility.small.bin";
	constexpr char host_file_path[] = "../static-genetic-algorithm/all-time-ok-hosts.json";

	std::vector<Entry> read_binary(const char* file_name);

	void read_host_json(const char* file_name, const char* binary_output_path);

	std::vector<HostInfo> read_host_binary();
}
