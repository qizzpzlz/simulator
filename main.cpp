#include "includes/parser.h"
#include "includes/scenario.h"
#include "includes/cluster.h"
#include "includes/cluster_simulation.h"
#include "spdlog/spdlog.h"
#include <fstream>

//#if defined 
#if defined _WIN32
const std::string SCENARIO_PATH = "../scenarios/";
#else
const std::string SCENARIO_PATH = "scenarios/";
#endif
const std::string HOSTS_FILE = "hardware_raw_initial_status.json";
const std::string SCENARIO_FILE = "scenario.json";
const int NUM_SCENARIO_LINES_LIMIT = 10000;

int main()
{
	// Temporary CLI for input files
	std::string scenario_path;
	std::string host_path;
	// int lines;
	// std::cout << "##Cluster Simulator" << std::endl;
	// std::cout << "Path to scenario file: ";
	// std::cin >> scenario_path;
	// std::cout << "Input the number of lines to read: ";
	// std::cin >> lines;
	// std::cout << "Path to host file: ";
	// std::cin >> host_path;
	
	ClusterSimulator::Scenario scenario;
	ClusterSimulator::Cluster cluster;
		
	// Parse the given scenario and the cluster from json files.
	if (!host_path.empty() && !scenario_path.empty())
	{
		ClusterSimulator::Parser::parse_scenario(&scenario, scenario_path);
		ClusterSimulator::Parser::parse_cluster(&cluster, host_path);
	}
	else
	{
		ClusterSimulator::Parser::parse_scenario(&scenario, SCENARIO_PATH + SCENARIO_FILE, NUM_SCENARIO_LINES_LIMIT);
		ClusterSimulator::Parser::parse_cluster(&cluster, SCENARIO_PATH + HOSTS_FILE);
	}

	// Start simulation
	ClusterSimulator::ClusterSimulation simulation{ scenario, cluster };
	simulation.run();

	// Print summary
	simulation.print_summary();



	//std::string current_working_dir(NPath);
	//std::cout << current_working_dir;	ClusterSimulator::ParseScenario(SCENARIO_PATH + "lines.json");

	//ClusterSimulator::ParseCluster(SCENARIO_PATH + HOSTS_FILE);
	//ClusterSimulator::parse_scenario(SCENARIO_PATH + SCENARIO_FILE);

	//test
	//double simulationTime = 7200;
	//int nodeNum = 8;
	//RandomAlgorithm algorithm{};

	//Simulation simulation{algorithm, simulationTime, nodeNum};

	//simulation.simulate();

	ClusterSimulator::ClusterSimulation::jobmart_file_.close();
}

