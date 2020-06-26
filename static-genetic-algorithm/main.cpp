#include "file_reader.h"
#include <filesystem>
#include <iostream>
#include <string>
#include "genetic_algorithm.h"
#include "utils.h"
#include "chromosome.h"
#include "argparse.hpp"

namespace genetic {
	std::vector<Entry> job_table;
	std::vector<HostInfo> host_table;
	std::vector<Host> host_prototypes;
	uint32_t LENGTH;
	uint32_t NUM_HOSTS;

	
void initialise_static_data()
{
	job_table = read_binary(binary_file_path);
	host_table = read_host_binary();
	host_prototypes.reserve(host_table.size());
	for (auto& info : host_table)
		host_prototypes.push_back(Host{ info.max_slots });

	LENGTH = job_table.size();
	NUM_HOSTS = host_table.size();
}
}

int main(int argc, char* argv[])
{
	// Parsing arguments
	argparse::ArgumentParser program("static-genetic-algorithm");
	program.add_argument("-p", "--population")
		.help("Optional population file to load. if provided, initial population is replaced with the given population.")
		.default_value(static_cast<std::string>(""));
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

	constexpr std::string_view best_chromosome_binary_name = "best_chromosome.bin";
	constexpr std::string_view last_population_binary_name = "last_population.bin";
	constexpr std::string_view epoch_record_csv_name = "records.csv";
	constexpr std::string_view summary_txt_name = "summary.txt";



	// Main driver code starts.
	std::default_random_engine rnd{};
	std::uniform_real_distribution<> real_dist{};
	std::stringstream output_buffer{};

	using namespace genetic;
	initialise_static_data();

	// Output configs
	using namespace std::filesystem;
	path output_dir_path = path{ OUTPUT_DIR_PATH };
	create_directory(output_dir_path);
	path best_chromosome_path = output_dir_path / best_chromosome_binary_name;
	path last_population_path = output_dir_path / last_population_binary_name;
	path epoch_record_path = output_dir_path / epoch_record_csv_name;
	path summary_path = output_dir_path / summary_txt_name;
	
	
	const std::function<double(Chromosome&)> get_fitness = [](Chromosome& c) { return c.fitness(); };

	auto pop_data = std::make_unique<Population>();
	auto offspring_data = std::make_unique<Offspring>();
	Population& population = *pop_data;
	Offspring& offspring = *offspring_data;
	std::vector<Chromosome> mutants;

	Chromosome* current_best;

	const auto time_before_initialisation = std::chrono::system_clock::now();

	if (std::string population_file_path{program.get<std::string>("--population")};
		!population_file_path.empty())
	{
		auto population_vector = load_population(population_file_path.c_str());
		if (NUM_POPULATION_TO_KEEP != population_vector.size())
		{
			std::cout << "Incompatible population." << std::endl;
			exit(0);
		}
		std::copy_n(population_vector.begin(), NUM_POPULATION_TO_KEEP, population.begin());
		calculate_fitness_parallel(population);

		generate_offspring_parallel(population, offspring);
		calculate_fitness_parallel(offspring);
	}
	else {
		generate_initial_population(population, offspring);
	}

	const auto time_after_initialisation = std::chrono::system_clock::now();

	select_survivors(population, offspring, mutants);

	// as chromosome gets bigger creating initial population takes long time, thus save
	save_population(population, last_population_path);

	for (auto iter = 0; iter < NUM_ITERATIONS; ++iter)
	{
		std::cout << "Epoch " << iter + 1 << std::endl;

		// Generate offspring.
		generate_offspring_parallel(population, offspring);

		calculate_fitness_parallel(offspring);

		// Mutation
		get_mutants(population, mutants, rnd);
		get_mutants(offspring, mutants, rnd);

		calculate_fitness_parallel(mutants);

		select_survivors(population, offspring, mutants);

		mutants.clear();

		Chromosome& best = *std::max_element(population.begin(), population.end(), [](Chromosome& a, Chromosome& b) { return a.fitness() < b.fitness(); });
		current_best = &best;

		if constexpr (CONSOLE_OUTPUT)
		{
			std::cout << "Best fitness: "
				<< std::fixed << best.fitness()
				<< " Age: " << best.age()
				<< " Type: " << Chromosome::type_strings[static_cast<int>(best.type())]
				<< std::endl;

			print_population_composition_by_types(population);
		}

		output_buffer << std::fixed << iter << ", " << best.fitness() << std::endl;

		for (auto& chromosome : population)
		{
			chromosome.increase_age();
		}

		if(iter % SAVE_INTERVAL == 0) {
			save_population(population, "last_population.bin");
		}
	}

	const auto time_finished = std::chrono::system_clock::now();


	// Save output files
	current_best->save(best_chromosome_path);
	save_population(population, last_population_path);
	save_epochs_record(output_buffer, epoch_record_path);
	save_summary_text(summary_path);

	const auto init_duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_after_initialisation - time_before_initialisation);
	const auto genetic_duration = std::chrono::duration_cast<std::chrono::minutes>(time_finished - time_after_initialisation);

	std::ofstream file(summary_path, std::ios::app | std::ios::out);
	file << "- Time elapsed for initialisation: " << init_duration.count() << " ms" << std::endl;
	file << "- Time elapsed for genetic algorithm: " << genetic_duration.count() << " min" << std::endl;
	file.close();
}
