#pragma once
#include "job.h"
#include "file_reader.h"
#include <cstddef>
#include <vector>
#include <sstream>
#include <string_view>

namespace genetic
{
	constexpr std::size_t RESERVED_ENTRIES_COUNT = 8000000;
	constexpr std::size_t NUM_POPULATION_TO_KEEP = 200;
	constexpr double SURVIVAL_RATIO = 0.2;
	constexpr double OFFSPRING_RATIO = 10;
	static_assert(OFFSPRING_RATIO >= 1);
	constexpr std::size_t NUM_INITIAL_POPULATION = NUM_POPULATION_TO_KEEP * OFFSPRING_RATIO;
	constexpr std::size_t NUM_OFFSPRING = NUM_POPULATION_TO_KEEP * (OFFSPRING_RATIO - 1);
	//constexpr std::size_t KEEP_SIZE = 15;
	constexpr std::size_t NUM_ITERATIONS = 200;
	constexpr std::size_t SAVE_INTERVAL = 10;
	//constexpr std::size_t AGE_UPPER_LIMIT = 0;
	constexpr double CROSSOVER_PROBABILITY = 1;
	constexpr double CROSSOVER_BALANCER = 0.5;

	constexpr double MUTATION_PROBABILITY = 0.5;
	//constexpr double MUTATION_BALANCER = 0.5;
	constexpr double GENE_MUTATION_PROBABILITY = 0.001;

	enum class CrossoverTypes { SinglePoint, MultiplePoint, Uniform };
	constexpr CrossoverTypes CROSSOVER_TYPE = CrossoverTypes::Uniform;
	constexpr char* CROSSOVER_TYPE_STRINGS[] = {"SinglePoint", "MultiplePoint", "Uniform"};

	constexpr bool CONSOLE_OUTPUT = true;

	constexpr std::string_view OUTPUT_DIR_PATH = "./out-genetic/";

	extern std::vector<Entry> job_table;
	extern std::vector<HostInfo> host_table;
	extern std::vector<Host> host_prototypes;
	extern uint32_t LENGTH;
	extern uint32_t NUM_HOSTS;


	//constexpr std::string_view parameters_summary = []
	//{
	//	constexpr char

	//
	//	return "";
	//}();
}
