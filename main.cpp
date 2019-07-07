#include "includes/Parser.h"
#include "includes/Scenario.h"
#include "includes/Cluster.h"
#include "includes/Cluster_Simulation.h"
#include "spdlog/spdlog.h"
#include <fstream>

//#if defined 
const std::string SCENARIO_PATH = "scenarios/";
//const std::string SCENARIO_PATH = std::filesystem::current_path().string() + "/scenarios/";
const std::string HOSTS_FILE = "hosts_final.json";
const std::string SCENARIO_FILE = "sliced_10000_Scenario_From_1000000_1000000.json";
//const std::string SCENARIO_PATH = "/scenarios/";

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
		scenario = ClusterSimulator::Parser::parse_scenario(scenario_path);
		cluster = ClusterSimulator::Parser::parse_cluster(host_path);
	}
	else
	{
		scenario = ClusterSimulator::Parser::parse_scenario(SCENARIO_PATH + SCENARIO_FILE, 5000);
		cluster = ClusterSimulator::Parser::parse_cluster(SCENARIO_PATH + HOSTS_FILE);
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

