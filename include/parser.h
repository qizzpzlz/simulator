#pragma once
#include <string>
#include <memory>

namespace ClusterSimulator
{
	class Scenario;
	class Cluster;
	namespace Parser
	{		
		void parse_scenario(Scenario* scenario, const std::string& file_path, int limit = -1);
		void parse_scenario_from_table(Scenario* scenario, const std::string& file_path);
		void parse_cluster(Cluster* cluster, const std::string& file_path);
		void parse_cluster_from_binary(Cluster* cluster, const std::string& file_path);
	}
}
