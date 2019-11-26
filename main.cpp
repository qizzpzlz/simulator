#include "includes/parser.h"
#include "includes/scenario.h"
#include "includes/cluster.h"
#include "includes/cluster_simulation.h"
#include "argparse.hpp"
#include <fstream>
#include <filesystem>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
const std::string SCENARIO_DIR_PATH = "../scenarios/";
#else
const std::string SCENARIO_DIR_PATH = "scenarios/";
#endif
const std::string HOSTS_FILE = "hardware_raw_initial_status.json";
const std::string SCENARIO_FILE = "scenario.json";
const int NUM_SCENARIO_LINES_LIMIT = -1;
const std::string LOG_DIR = "logs/";

namespace fs = std::filesystem;

int main(int argc, char *argv[])
{

	argparse::ArgumentParser program("cluster-simulator");
	program
		.add_argument("-p", "--path")
		.required()
		.help("specify the path to scenario directory.")
		.default_value(SCENARIO_DIR_PATH);
	program.add_argument("-c", "--count")
		.help("number of items to read in the given scenario.")
		.default_value(NUM_SCENARIO_LINES_LIMIT)
		.action([](const std::string& value) { return std::stoi(value); });

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
	const std::string scenario_path{ scenario_dir_path + SCENARIO_FILE };
	const std::string host_path{ scenario_dir_path + HOSTS_FILE };

	// Create logs directory
	fs::create_directory(fs::path{ LOG_DIR });

	
	ClusterSimulator::Scenario scenario;
	ClusterSimulator::Cluster cluster;
		
	// Parse the given scenario and the cluster from json files.
	ClusterSimulator::Parser::parse_scenario(&scenario, scenario_path, program.get<int>("--count"));
	ClusterSimulator::Parser::parse_cluster(&cluster, host_path);


	// Start simulation
	ClusterSimulator::ClusterSimulation simulation{ scenario, cluster, *ClusterSimulator::QueueAlgorithms::OLB };

	

	simulation.run();

	// Print summary
	simulation.print_summary();

	ClusterSimulator::ClusterSimulation::jobmart_file_.close();
}

