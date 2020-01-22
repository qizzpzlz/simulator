#include "genetic_algorithm.h"
#include "chromosome.h"
#include <chrono>
#include <iostream>

namespace genetic
{
	void generate_initial_population()
	{
		auto timer = std::chrono::system_clock::now();

		
		//std::vector<Chromosome*> population;
		std::array<Chromosome, NUM_INITIAL_POPULATION> population;
	
		for (int i = 0; i < NUM_INITIAL_POPULATION; ++i)
			population[i].make_random();

		std::array<double, NUM_INITIAL_POPULATION> fitness_values;

		for (int i = 0; i < NUM_INITIAL_POPULATION; ++i)
			fitness_values[i] = population[i].calculate_fitness();

		std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - timer;

		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
		
		std::cout << "Time elapsed for " << NUM_INITIAL_POPULATION << " chromosomes: " << milliseconds.count() << "ms" << std::endl;
	}
}
