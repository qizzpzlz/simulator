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
	
	
	using namespace genetic;
	const std::function<double(Chromosome&)> get_fitness = [](Chromosome& c) { return c.fitness(); };
	
	auto pop_data = std::make_unique<Population>();
	auto offspring_data = std::make_unique<Offspring>();
	Population& population = *pop_data;
	Offspring& offspring = *offspring_data;
	std::vector<Chromosome> mutants;

	Chromosome* current_best;
	
	generate_initial_population(population);

	for (auto iter = 0; iter < NUM_ITERATIONS; ++iter)
	{
		std::cout << "Epoch " << iter + 1 << std::endl;

		// Generate offspring.
		//for (int i = 0; i < NUM_OFFSPRING; ++i)
		//{
		//	auto roll = real_dist(rnd);
		//	if (roll < CROSSOVER_PROBABILITY)
		//	{
		//		// Parent selection.
		//		auto parent_indices = get_weighted_random_items<2>(population, get_fitness, rnd);

		//		//offspring[i] = std::move(population[parent_indices[0]].crossover(population[parent_indices[1]]));
		//		offspring[i].crossover(population[parent_indices[0]], population[parent_indices[1]]);
		//	}
		//	else
		//	{
		//		// Random children.
		//		offspring[i].make_random();
		//	}
		//}
		generate_offspring_parallel(population, offspring);

		calculate_fitness_parallel(offspring);
		//std::future<void> offspring_fitness_async(std::async([&] { return calculate_fitness_parallel(offspring); }));
		

		// Mutation
		get_mutants(population, mutants, rnd);
		get_mutants(offspring, mutants, rnd);

		//std::future<void> mutants_fitness_async(std::async([&] {return calculate_fitness_parallel(mutants); }));
		calculate_fitness_parallel(mutants);

		//offspring_fitness_async.get();
		//mutants_fitness_async.get();

		select_survivors(population, offspring, mutants);

		mutants.clear();

		Chromosome& best = *std::max_element(population.begin(), population.end(), [](Chromosome& a, Chromosome& b) { return a.fitness() < b.fitness(); });
		current_best = &best;

		std::cout << "Best fitness: "
				  << std::fixed << best.fitness()
				  << " Age: " << best.age()
				  << " Type: " << Chromosome::type_strings[static_cast<int>(best.type())]
				  << std::endl;

		for (auto& chromosome : population)
		{
			chromosome.increase_age();
		}
	}

	current_best->save("best_chromosome.bin");
}
