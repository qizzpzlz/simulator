#include "file_reader.h"
#include <filesystem>
#include <iostream>
#include "genetic_algorithm.h"
#include "utils.h"
#include "chromosome.h"
#include <future>

int main(int argc, char* argv[])
{
	std::default_random_engine rnd{};
	std::uniform_real_distribution<> real_dist{};
	std::stringstream output_buffer{};
	
	using namespace genetic;
	const std::function<double(Chromosome&)> get_fitness = [](Chromosome& c) { return c.fitness(); };
	
	auto pop_data = std::make_unique<Population>();
	auto offspring_data = std::make_unique<Offspring>();
	Population& population = *pop_data;
	Offspring& offspring = *offspring_data;
	std::vector<Chromosome> mutants;

	Chromosome* current_best;

	const auto time_before_initialisation = std::chrono::system_clock::now();
	
	generate_initial_population(population, offspring);

	const auto time_after_initialisation = std::chrono::system_clock::now();

	select_survivors(population, offspring, mutants);

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
	}


	// Save output files
	current_best->save("best_chromosome.bin");
	save_population(population, "last_population.bin");
	save_epochs_record(output_buffer, "records.csv");
	save_summary_text("summary.txt");
}
