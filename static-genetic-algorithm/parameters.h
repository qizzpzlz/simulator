#pragma once
#include "job.h"
#include "file_reader.h"
#include <cstddef>
#include <vector>

namespace genetic
{
	constexpr char binary_file_path[] = "../static-genetic-algorithm/job-eligibility.small.bin";

	constexpr std::size_t NUM_POPULATION_TO_KEEP = 200;
	constexpr double SURVIVAL_RATIO = 0.2;
	constexpr double OFFSPRING_RATIO = 5;
	static_assert(OFFSPRING_RATIO >= 1);
	
	constexpr std::size_t NUM_OFFSPRING = NUM_POPULATION_TO_KEEP * (OFFSPRING_RATIO - 1);
	//constexpr std::size_t KEEP_SIZE = 15;
	constexpr std::size_t NUM_ITERATIONS = 100;
	constexpr std::size_t AGE_UPPER_LIMIT = 3;
	constexpr double CROSSOVER_PROBABILITY = 1;
	constexpr double CROSSOVER_BALANCER = 0.5;
	
	constexpr double MUTATION_PROBABILITY = 0.2;
	constexpr double MUTATION_BALANCER = 0.5;
	constexpr double GENE_MUTATION_PROBABILITY = 0.5;

	enum class CrossoverTypes { SinglePoint, MultiplePoint, Uniform };
	constexpr CrossoverTypes CROSSOVER_TYPE = CrossoverTypes::Uniform;

	inline static const std::vector<Entry> job_table = read_binary(binary_file_path);
	inline static std::vector<HostInfo> host_table = read_host_binary();
	inline static const std::vector<Host> host_prototypes = []
	{
		std::vector<Host> result;
		result.reserve(host_table.size());
		for (auto& info : host_table)
		{
			result.push_back(Host{ info.max_slots });
		}

		return result;
	}();
	inline static const unsigned short LENGTH = job_table.size();
	inline static const std::size_t NUM_HOSTS = host_table.size();
}
