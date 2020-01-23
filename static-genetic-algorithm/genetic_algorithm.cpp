#include "genetic_algorithm.h"
#include "chromosome.h"
#include <chrono>
#include <iostream>
#include <execution>

namespace genetic
{
	void generate_initial_population(Population& population)
	{
		auto timer = std::chrono::system_clock::now();

		
		//std::vector<Chromosome*> population;
		//std::array<Chromosome, NUM_POPULATION_TO_KEEP> population;
	
		for (int i = 0; i < NUM_POPULATION_TO_KEEP; ++i)
			population[i].make_random();

		std::array<double, NUM_POPULATION_TO_KEEP> fitness_values;

		for (int i = 0; i < NUM_POPULATION_TO_KEEP; ++i)
			fitness_values[i] = population[i].calculate_fitness();

		std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - timer;

		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
		
		std::cout << "Time elapsed for " << NUM_POPULATION_TO_KEEP << " chromosomes: " << milliseconds.count() << "ms" << std::endl;
	}

	void select_survivors(Population& population, Offspring& offspring, std::vector<Chromosome>& mutants)
	{
		const std::size_t total_count = NUM_POPULATION_TO_KEEP + NUM_OFFSPRING + mutants.size();

		// Warning: thread unsafe
		static std::vector<TemporaryChromosomeData> temp_list;
		static std::vector<TemporaryChromosomeData> temp_population(NUM_POPULATION_TO_KEEP);

		if (temp_list.size() < total_count)
			temp_list = std::vector<TemporaryChromosomeData>(total_count);

		std::size_t k = 0;
		for (std::size_t i = 0; i < NUM_POPULATION_TO_KEEP; ++i)
			temp_list[k++] = TemporaryChromosomeData{ population[i].fitness(), i, Locations::Population };
		for (std::size_t i = 0; i < NUM_OFFSPRING; ++i)
			temp_list[k++] = TemporaryChromosomeData{ offspring[i].fitness(), i, Locations::Offspring };
		for (std::size_t i = 0; i < mutants.size(); ++i)
			temp_list[k++] = TemporaryChromosomeData{ mutants[i].fitness(), i, Locations::Mutants };

		std::sort(temp_list.begin(), temp_list.begin() + total_count,
			[](TemporaryChromosomeData& a, TemporaryChromosomeData& b)
			{
				return a.fitness > b.fitness;
			});

		const auto end = temp_list.cbegin() + NUM_POPULATION_TO_KEEP;

		std::size_t i{ 0 };
		for (auto it = temp_list.cbegin(); it != end; ++it)
		{
			auto location = it->location;
			if (location == Locations::Population)
			{
				temp_population[i++] = *it;
			}
		}

		std::sort(temp_population.begin(), temp_population.begin() + i,
			[](TemporaryChromosomeData& a, TemporaryChromosomeData& b)
			{
				return a.index < b.index;
			});

		k = 0;
		for (auto it = temp_population.begin(); it != temp_population.begin() + i; ++it)
		{
			std::swap(population[it->index], population[k++]);
		}
		
		for (auto it = temp_list.cbegin(); it != end; ++it)
		{
			auto location = it->location;
			if (location == Locations::Offspring)
				population[k++] = offspring[it->index];
			else if (location == Locations::Mutants)
				population[k++] = mutants[it->index];
		}
	}
}
