#pragma once
#include "Parser.h"
#include "Scenario.h"
#include "Cluster.h"

namespace ClusterSimulator
{
	namespace Parser
	{
		Scenario parse_scenario(const std::string& file_path, int limit = -1);
		Cluster parse_cluster(const std::string& file_path);
	}

}
