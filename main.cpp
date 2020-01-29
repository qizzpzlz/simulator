#define SPDLOG_COMPILED_LIB
#include "parser.h"
#include "scenario.h"
#include "cluster.h"
#include "cluster_simulation.h"
#include "argparse.hpp"
#include "queue_algorithm.h"
#include <filesystem>

const std::string SCENARIO_DIR_PATH =
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
"../scenarios/";
#else
"scenarios/";
#endif
constexpr char HOSTS_FILE[] = "hardware_raw_initial_status.json";
constexpr char SCENARIO_FILE[] = "scenario.json";
const int NUM_SCENARIO_LINES_LIMIT = -1;
constexpr std::string_view LOG_DIR = "logs/";

namespace fs = std::filesystem;



int main(int argc, char *argv[])
{
	argparse::ArgumentParser program("cluster-simulator");
	program
		.add_argument("-p", "--path")
		//.required()
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
	create_directory(fs::path{ LOG_DIR });

	using namespace ClusterSimulator;
	
	Scenario scenario;
	ClusterSimulator::Cluster cluster;
		
	// Parse the given scenario and the cluster from json files.
	//Parser::parse_scenario(&scenario, scenario_path, program.get<int>("--count"));
	//Parser::parse_cluster(&cluster, host_path);

	Parser::parse_scenario_from_table(&scenario, "../static-genetic-algorithm/job-eligibility.small.bin");
	Parser::parse_cluster_from_binary(&cluster, "hosts.bin");

	//MinMinAlgorithm minmin(cluster.size());

	// Start simulation
	ClusterSimulation simulation{ scenario, cluster, *QueueAlgorithms::MinMin};

	simulation.run();

	// Print summary
	simulation.print_summary();
}

