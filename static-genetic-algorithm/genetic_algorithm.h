#pragma once
#include "parameters.h"
#include "chromosome.h"
#include <array>
#include <execution>
#include "utils.h"
#include <iostream>

namespace genetic
{
	using Population = std::array<Chromosome, NUM_POPULATION_TO_KEEP>;
	using Offspring = std::array<Chromosome, NUM_OFFSPRING>;

	void generate_initial_population(Population& population);

	template<std::size_t N, typename RND>
	void get_mutants(const std::array<Chromosome, N>& population, std::vector<Chromosome>& buffer, RND& generator)
	{
		std::uniform_real_distribution<> real_dist;
		for (const Chromosome& chromosome : population)
		{
			if (chromosome.type() == Chromosome::Type::RANDOM) continue;
			
			auto roll = real_dist(generator);
			if (roll < MUTATION_PROBABILITY)
			{
				buffer.push_back(chromosome.mutate());
			}
		}
	}

	enum class Locations
	{
		Population, Offspring, Mutants
	};

	struct TemporaryChromosomeData
	{
		double fitness;
		std::size_t index;
		Locations location;
	};

	void select_survivors(Population& population, Offspring& offspring, std::vector<Chromosome>& mutants);

	template <typename Chromosomes>
	void calculate_fitness_parallel(Chromosomes& chromosomes)
	{
		std::for_each(std::execution::par_unseq, chromosomes.begin(), chromosomes.end(), []
		(Chromosome& c) {
				c.calculate_fitness();
			});
	}

	inline void generate_offspring_parallel(Population& population, Offspring& offspring)
	{
		

		std::for_each(std::execution::par, offspring.begin(), offspring.end(), [&]
		(Chromosome& c)
		{
			static thread_local std::default_random_engine rnd{};
			//thread_local std::default_random_engine rnd{};
			std::uniform_real_distribution<> dist{};
			//std::uniform_int_distribution<> dist(0, NUM_POPULATION_TO_KEEP - 1);
			const std::function<double(Chromosome&)> get_fitness = [](Chromosome& c) { return c.fitness(); };
			auto roll = dist(rnd);
			if (roll < CROSSOVER_PROBABILITY)
			{
				// Parent selection.
				auto parent_indices = get_weighted_random_items<2>(population, get_fitness, rnd);
				c.crossover(population[parent_indices[0]], population[parent_indices[1]]);

				//c.crossover(population[dist(rnd)], population[dist(rnd)]);
			}
			else
			{
				// Random children.
				c.make_random();
			}
		});
	}
}
