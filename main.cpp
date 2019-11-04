#include "includes/parser.h"
#include "includes/scenario.h"
#include "includes/cluster.h"
#include "includes/cluster_simulation.h"
#include "argparse.hpp"
#include <string>
#include <sstream>

const std::string SCENARIO_DIR_PATH = "scenarios/";
const std::string HOSTS_FILE = "hardware_raw_initial_status.json";
const std::string SCENARIO_FILE = "scenario.json";
const int NUM_SCENARIO_LINES_LIMIT = -1;

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
		.default_value(-1);

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
	
	ClusterSimulator::Scenario scenario;
	ClusterSimulator::Cluster cluster;
		
	// Parse the given scenario and the cluster from json files.
	ClusterSimulator::Parser::parse_scenario(&scenario, scenario_path, 100);
	ClusterSimulator::Parser::parse_cluster(&cluster, host_path);

	// Start simulation
	ClusterSimulator::ClusterSimulation simulation{ scenario, cluster, *ClusterSimulator::QueueAlgorithms::OLB};
	simulation.run();

	// Print summary
	simulation.print_summary();

	//ClusterSimulator::ClusterSimulation::jobmart_file_.close();
}

