#define SPDLOG_COMPILED_LIB
#include "parser.h"
#include "scenario.h"
#include "cluster.h"
#include "cluster_simulation.h"
#include "argparse.hpp"
#include "queue_algorithm.h"
#include <filesystem>
#include "test.h"
#include <array>

namespace fs = std::filesystem;
using namespace cs;


int main(int argc, char *argv[])
{
	argparse::ArgumentParser program("cluster-simulator");
	program
		.add_argument("-p", "--path")
		.help("specify the path to scenario directory.")
		.default_value(std::string(config::SCENARIO_FILE_PATH_STRING));
	program.add_argument("--table-binary-path")
		.help("Path to the job table binary file")
		.default_value(std::string("table.bin"))
		.required();
	program.add_argument("-c", "--count")
		.help("number of items to read in the given scenario.")
		.default_value(-1)
		.action([](const std::string& value) { return std::stoi(value); });
	program.add_argument("-j", "--use-json")
		.help("Use json scenario instead of binary job table.")
		.default_value(false)
		.implicit_value(true);
	program.add_argument("-a", "--algorithm")
		.help("Queue algorithm to be used. (OLB, MCT or MinMin)")
		.required()
		.default_value(QueueAlgorithms::MinMin)
		.action([](const std::string& value)->const QueueAlgorithm*
				{
					if (value == "OLB") return QueueAlgorithms::OLB;
					if (value == "MCT") return QueueAlgorithms::MCT;
					if (value == "MinMin") return QueueAlgorithms::MinMin;
					throw std::runtime_error("Queue algorithm " + value + " is invalid.");
				});
	program.add_argument("--binary-old")
		.help("Use generated allocation binary for queue algorithm (simple format).")
		.default_value(false)
		.implicit_value(true);
	program.add_argument("--alloc-binary-path")
		.help("Path to the allocation binary file")
		.default_value(std::string("allocs.bin"))
		.required();
	program.add_argument("--use-weight-csv")
		.help("Use weights as a queue algorithm.")
		.default_value(true)
		.implicit_value(true);
	program.add_argument("--weight-csv-path")
		.help("Path to the weight csv file.")
		.default_value(std::string("weights.csv"))
		.required();

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
	if (!fs::exists(scenario_dir_path))
	{
		std::cerr << "Scenario directory is missing" << std::endl;
		exit(1);
	}
	
	const std::string scenario_path{ scenario_dir_path + std::string(config::JSON_SCENARIO_FILE_NAME) };
	const std::string host_path{ scenario_dir_path + std::string(config::JSON_HOST_FILE_NAME) };

	// Create logs directory
	create_directory(fs::path{ config::LOG_DIR_PATH_STRING });
	
	Scenario scenario;
	cs::Cluster cluster;
	
	if (program["--use-json"] == false)
	{
		// TODO: parse paths from the argument parser.
		//constexpr std::string_view scenario_binary_path = "../static-genetic-algorithm/job-eligibility.small.bin";
		std::string scenario_binary_path = program.get<std::string>("--table-binary-path");
		constexpr std::string_view host_binary_path = "hosts.bin";
		
		parser::parse_scenario_from_table(&scenario, scenario_binary_path);
		parser::parse_cluster_from_binary(&cluster, host_binary_path);
	}
	else
	{
		// Parse the given scenario and the cluster from json files.
		parser::parse_scenario(&scenario, scenario_path, program.get<int>("--count"));
		parser::parse_cluster(&cluster, host_path);
	}

	std::string binary_allocation_file_name = program.get<std::string>("--alloc-binary-path");
	std::string binary_allocation_file_path = scenario_dir_path + std::string(binary_allocation_file_name);

	// Start simulation
	std::shared_ptr<QueueAlgorithm> algorithm_storage;
	std::unique_ptr<ClusterSimulation> simulation;
	if (program["--binary-old"] == false)
	{
		const QueueAlgorithm* algorithm;
		if (program["--use-weight-csv"] == true)
		{
			auto weights = 
				parser::parse_weights<WeightBasedGreedySelection::num_weights>(program.get<std::string>("--weight-csv-path"));

			algorithm_storage = std::make_shared<WeightBasedGreedySelection>(weights);
			algorithm = algorithm_storage.get();
		}
		else
		{
			algorithm = program.get<const QueueAlgorithm*>("--algorithm");
		}
		
		simulation = std::make_unique<ClusterSimulation>(scenario, cluster, *algorithm);
	}
	else
		simulation = std::make_unique<ClusterSimulation>(cluster, scenario, binary_allocation_file_path, true);

	simulation->run();

	// Print summary
	simulation->print_summary();

	return 0;
}

