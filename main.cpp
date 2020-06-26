#define SPDLOG_COMPILED_LIB
#include "parser.h"
#include "scenario.h"
#include "cluster.h"
#include "cluster_simulation.h"
#include "argparse.hpp"
#include "queue_algorithm.h"
#include <filesystem>

namespace fs = std::filesystem;
using namespace cs;


int main(int argc, char *argv[])
{
	argparse::ArgumentParser program("cluster-simulator");
	program
		.add_argument("-p", "--path")
		.help("specify the path to scenario directory.")
		.default_value(std::string(config::SCENARIO_FILE_PATH_STRING));
	program.add_argument("-c", "--count")
		.help("number of items to read in the given scenario.")
		.default_value(-1)
		.action([](const std::string& value) { return std::stoi(value); });
	program.add_argument("-t", "--use-table")
		.help("Use binary scenario table instead of json scenario file.")
		.default_value(true)
		.implicit_value(true);

	try
	{
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err)
	{
		std::cout << err.what() << std::endl;
		program.print_help();
		exit(0);
	}

	const auto scenario_dir_path{ program.get<std::string>("--path") };
	const std::string scenario_path{ scenario_dir_path + std::string(config::JSON_SCENARIO_FILE_NAME) };
	const std::string host_path{ scenario_dir_path + std::string(config::JSON_HOST_FILE_NAME) };

	// Create logs directory
	create_directory(fs::path{ config::LOG_DIR_PATH_STRING });
	
	Scenario scenario;
	cs::Cluster cluster;
	
	if (program["--use-table"] == true)
	{
		// TODO: parse paths from the argument parser.
		constexpr std::string_view scenario_binary_path = "../static-genetic-algorithm/job-eligibility.small.bin";
		constexpr std::string_view host_binary_path = "hosts.bin";
		
		Parser::parse_scenario_from_table(&scenario, scenario_binary_path);
		Parser::parse_cluster_from_binary(&cluster, host_binary_path);
	}
	else
	{
		// Parse the given scenario and the cluster from json files.
		Parser::parse_scenario(&scenario, scenario_path, program.get<int>("--count"));
		Parser::parse_cluster(&cluster, host_path);
	}

	// Start simulation
	ClusterSimulation simulation{ scenario, cluster, *QueueAlgorithms::MinMin};

	simulation.run();

	// Print summary
	simulation.print_summary();

	return 0;
}

