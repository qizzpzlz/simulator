#pragma once
#include <string>
#include <memory>
#include <iostream>
#include <array>
#include <sstream>
#include <string>
#include <fstream>

namespace cs
{
	class Scenario;
	class Cluster;
	namespace parser
	{		
		void parse_scenario(Scenario* scenario, const std::string& file_path, int limit = -1);
		void parse_scenario_from_table(Scenario* scenario, std::string_view file_path);
		void parse_cluster(Cluster* cluster, const std::string& file_path);
		void parse_cluster_from_binary(Cluster* cluster, std::string_view file_path);

		template <unsigned N>
		auto parse_weights(std::string_view file_path)->std::array<double, N>
		{
			constexpr unsigned num_weights = 5;

			std::ifstream file{ file_path.data() };
			if (!file.is_open())
			{
				std::cerr << "Could not open file " << file_path << '.' << std::endl;
				exit(1);
			}

			std::array<double, num_weights> weights;
			std::string line;
			std::getline(file, line);

			std::stringstream ss{ line };
			for (auto i = 0; i < num_weights; ++i)
			{
				ss >> weights[i];

				if (ss.peek() == ',')
					ss.ignore();
			}

			file.close();

			return weights;
		}
	}
}
