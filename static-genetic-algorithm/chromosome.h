#pragma once
#include "parameters.h"
#include "host.h"
#include <vector>
#include <random>

namespace genetic
{
	struct Entry;

	class Chromosome
	{
	public:
		using const_iterator = std::vector<uint16_t>::const_iterator;
		enum class Type{ RANDOM, CROSSOVER, MUTATION };
		inline static std::string type_strings[] = { "Random", "Crossover", "Mutation" };

		Chromosome() : data_(LENGTH), hosts_(host_prototypes) {}

		[[nodiscard]] Host& get_host(std::size_t index)
		{
			return hosts_[index];
		}
		
		[[nodiscard]] const_iterator begin() const { return data_.begin(); }
		[[nodiscard]] const_iterator end() const { return data_.end(); }

		[[nodiscard]] uint16_t operator[](unsigned index) const
		{
			return data_[index];
		}

		[[nodiscard]] static Chromosome create_random();

		void make_random();

		[[nodiscard]] Chromosome mutate() const;

		[[nodiscard]] Chromosome crossover(const Chromosome& other) const;

		void crossover(const Chromosome& p1, const Chromosome& p2);

		double calculate_fitness();

		double fitness()
		{
			return calculate_fitness();
		}

		[[nodiscard]] std::size_t age() const noexcept { return age_; }
		void increase_age() { ++age_; }

		[[nodiscard]] Type type() const noexcept { return type_; }

		void save(const char* file_path) const
		{
			std::filebuf output_stream;
			output_stream.open(file_path, std::ios::out | std::ios::binary);

			for (auto i = 0; i < LENGTH; ++i)
			{
				output_stream.sputn(reinterpret_cast<const char*>(&data_[i]), sizeof(uint16_t));
			}
		}
		
	private:
		explicit Chromosome(uint16_t length)
			: data_(length)
			, hosts_(host_prototypes)
		{
			
		}

		void reset(Type type)
		{
			type_ = type;
			hosts_ = host_prototypes;
			fitness_cache_ = 1;
			age_ = 0;
		}
		
		std::vector<uint16_t> data_;
		std::vector<Host> hosts_;
		double fitness_cache_ = 1;
		std::size_t age_ = 0;
		Type type_ ;

		inline static thread_local std::default_random_engine rnd{};
	};
}