#pragma once
#include <string>

namespace ClusterSimulator
{
	class Scenario;
	class Cluster;
	namespace Parser
	{
		Scenario parse_scenario(const std::string& file_path, int limit = -1);
		Cluster parse_cluster(const std::string& file_path);
	}

}
