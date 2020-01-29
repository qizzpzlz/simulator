#pragma once
#include "parameters.h"
#include "chromosome.h"
#include <array>
#include "utils.h"
#include <iostream>
#include <iomanip>
#include <omp.h>

namespace genetic
{
	using Population = std::array<Chromosome, NUM_POPULATION_TO_KEEP>;
	using Offspring = std::array<Chromosome, NUM_OFFSPRING>;

	void generate_initial_population(Population& population, Offspring& offspring);

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

	template <std::size_t N>
	void get_mutants_parallel(const std::array<Chromosome, N>& chromosomes, std::vector<Chromosome>& buffer)
	{
		int i;
		const std::uniform_real_distribution<> real_dist;
//#pragma omp parallel for
//		for (i = 0; i < chromosomes.size(); ++i)
//		{
//			auto roll = real_dist(Chromosome::get_random_engine());
//			if (roll < MUTATION_PROBABILITY)
//			{
//				
//			}
//		}
		
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
		//std::for_each(std::execution::par_unseq, chromosomes.begin(), chromosomes.end(), []
		//(Chromosome& c) {
		//		c.calculate_fitness();
		//	});

		int i;
#pragma omp parallel for
		for (i = 0; i < chromosomes.size(); ++i)
		{
			chromosomes[i].calculate_fitness();
		}
	}

	inline void generate_offspring_parallel(Population& population, Offspring& offspring)
	{
		const std::function<double(const Chromosome&)> get_fitness = [](const Chromosome& c) { return c.fitness(); };
		int i;
#pragma omp parallel for
		for (i = 0; i < offspring.size(); ++i)
		{
			auto& c = offspring[i];

			auto parent_indices = get_weighted_random_items<2>(population, get_fitness, Chromosome::get_random_engine());
			c.crossover(population[parent_indices[0]], population[parent_indices[1]]);
		}
	}

	inline void print_population_composition_by_types(Population& population)
	{
		std::array<std::size_t, Chromosome::NUM_TYPES> record{};
		for (auto& i : population)
		{
			++record[static_cast<int>(i.type())];
		}

		std::cout << "Population Compositions: ";
		for (int i = 0; i < Chromosome::NUM_TYPES; ++i)
		{
			if (record[i] == 0) continue;
			
			std::cout
				<< Chromosome::type_strings[i] << ": " << std::fixed << std::setprecision(2) <<
				record[i] / static_cast<double>(NUM_POPULATION_TO_KEEP) * 100 << "% ";
		}
		std::cout << std::endl;
	}

	void save_population(Population& population, const char* file_path);
	void save_epochs_record(std::stringstream& ss, const char* file_path);
	inline void save_summary_text(const char* file_path)
	{
		std::ofstream output_stream(file_path);
		output_stream << "###### Summary ######\n"
			<< "- NUM_POPULATION_TO_KEEP: " << NUM_POPULATION_TO_KEEP << "\n"
			<< "- OFFSPRING_RATIO: " << OFFSPRING_RATIO << "\n"
			<< "- NUM_ITERATIONS: " << NUM_ITERATIONS << "\n"
			<< "- CROSSOVER_PROBABILITY: " << CROSSOVER_PROBABILITY << "\n"
			<< "- CROSSOVER_TYPE: " << CROSSOVER_TYPE_STRINGS[static_cast<int>(CROSSOVER_TYPE)] << "\n"
			<< "- MUTATION_PROBABILITY: " << MUTATION_PROBABILITY << "\n"
			<< "- GENE_MUTATION_PROBABILITY: " << GENE_MUTATION_PROBABILITY << "\n"
			;
		output_stream.close();
	}

	std::vector<Chromosome> load_population(const char* file_path);
}
