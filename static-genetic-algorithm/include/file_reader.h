#pragma once
#include <string>
#include <fstream>
#include <vector>
#include "host.h"

//#include "../include/host.h"
//#include "../include/cluster.h"

namespace genetic
{
	std::vector<Entry> read_binary(const char* file_name);

	void read_host_json(const char* file_name, const char* binary_output_path);

	std::vector<HostInfo> read_host_binary();
	
}
