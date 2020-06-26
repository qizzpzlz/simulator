#include "genetic_algorithm.h"
#include "chromosome.h"
#include <chrono>
#include <iostream>

namespace genetic
{
	void generate_initial_population(Population& population, Offspring& offspring)
	{
		auto timer = std::chrono::system_clock::now();

		//std::vector<Chromosome*> population;
		//std::array<Chromosome, NUM_POPULATION_TO_KEEP> population;

		#pragma omp parallel for
		for (long long i = 0; i < population.size(); ++i)
		{
			population[i].make_random();
			population[i].calculate_fitness();
		}

		#pragma omp parallel for
		for (long long i = 0; i < offspring.size(); ++i)
		{
			offspring[i].make_random();
			offspring[i].calculate_fitness();
		}

		std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - timer;

		const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

		std::cout << "Time elapsed for " << NUM_INITIAL_POPULATION << " chromosomes: " << milliseconds.count() << "ms" << std::endl;
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

	void save_population(Population& population, const char* file_path)
	{
		std::filebuf output_stream;
		output_stream.open(file_path, std::ios::out | std::ios::binary);

		std::size_t pop_size = population.size();
		uint32_t chromosome_length = LENGTH;

		output_stream.sputn(reinterpret_cast<char*>(&pop_size), sizeof(std::size_t));
		output_stream.sputn(reinterpret_cast<char*>(&chromosome_length), sizeof(uint32_t));

		for (auto& c : population)
		{
			output_stream.sputn(reinterpret_cast<const char*>(c.get_raw_data()), sizeof(uint16_t) * LENGTH);
		}
	}

	void save_epochs_record(std::stringstream& ss, const char* file_path)
	{
		std::ofstream output_stream;
		output_stream.open(file_path);
		output_stream << ss.rdbuf();
		output_stream.close();
	}

	std::vector<Chromosome> load_population(const char* file_path)
	{
		auto file = std::ifstream(file_path, std::ios::binary | std::ios::ate);

		if (!file)
			throw std::runtime_error("");

		file.seekg(0, std::ifstream::end);
		const size_t size = file.tellg();
		file.seekg(0, std::ifstream::beg);

		if (size == 0) return std::vector<Chromosome>{};

		std::vector<std::byte> buffer(size);

		if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
			throw std::runtime_error("");

		std::byte* init_ptr = buffer.data();
		std::byte* current = init_ptr;

		std::size_t pop_size = *reinterpret_cast<std::size_t*>(current);
		current += sizeof(std::size_t);

		std::cout << "Loading population of " << pop_size << " Chromosomes..." << std::endl;

		const uint32_t chromosome_length = *reinterpret_cast<uint32_t*>(current);
		const std::size_t c_length_in_bytes = chromosome_length * sizeof(uint16_t);
		current += sizeof(uint32_t);

		std::vector<Chromosome> result;
		result.reserve(pop_size);
		for (int i = 0; i < pop_size; ++i)
		{
			result.emplace_back(reinterpret_cast<uint16_t*>(current), chromosome_length);
			current += c_length_in_bytes;
		}

		std::cout << "Successfully loading " << pop_size << " Chromosomes with the length " << chromosome_length << std::endl;

		return result;
	}
}
