#include "chromosome.h"

namespace genetic
{
	Chromosome Chromosome::create_random()
	{
		static thread_local std::default_random_engine rnd;
		std::uniform_int_distribution<> dist(0, NUM_HOSTS);
		Chromosome chromosome(LENGTH);
		auto& data = chromosome.data_;
		for (int i = 0; i < LENGTH; ++i)
			data[i] = dist(rnd);
		return chromosome;
	}

	void Chromosome::make_random()
	{
		reset(Type::RANDOM);
		for (int i = 0; i < LENGTH; ++i)
		{
			auto& eligible_hosts = job_table[i].hosts;
			std::uniform_int_distribution<> dist(0, eligible_hosts.size() - 1);
			data_[i] = eligible_hosts[dist(rnd)].value;
		}
	}

	Chromosome Chromosome::mutate() const
	{
		const std::uniform_real_distribution<> real_distribution{};

		Chromosome child{};
		child.type_ = Type::MUTATION;
		std::copy(data_.begin(), data_.end(), child.data_.begin());
		for (int i = 0; i < LENGTH; ++i)
		{
			auto roll = real_distribution(rnd);
			if (roll >= GENE_MUTATION_PROBABILITY) continue;
			
			auto& eligible_hosts = job_table[i].hosts;
			std::uniform_int_distribution<> dist(0, eligible_hosts.size() - 1);
			child.data_[i] = eligible_hosts[dist(rnd)].value;
		}

		return child;
	}

	Chromosome Chromosome::crossover(const Chromosome& other) const
	{
		Chromosome child{};
		child.type_ = Type::CROSSOVER;


		if constexpr (CROSSOVER_TYPE == CrossoverTypes::Uniform)
		{
			const std::uniform_real_distribution<> real_distribution{};

			for (int i = 0; i < LENGTH; ++i)
			{
				auto roll = real_distribution(rnd);
				child.data_[i] = roll < CROSSOVER_BALANCER
					? data_[i]
					: other.data_[i];
			}
		}
		else if constexpr (CROSSOVER_TYPE == CrossoverTypes::SinglePoint)
		{
			std::uniform_int_distribution<> int_distribution(0, LENGTH - 1);
			auto point = int_distribution(rnd);
			std::copy(data_.begin(), data_.begin() + point, child.data_.begin());
			std::copy(other.data_.begin() + point, other.data_.end(), child.data_.begin() + point);
		}
		else
		{
			//static_assert(true);
		}
		
		return child;
	}

	void Chromosome::crossover(const Chromosome& p1, const Chromosome& p2)
	{
		reset(Type::CROSSOVER);
		if constexpr (CROSSOVER_TYPE == CrossoverTypes::Uniform)
		{
			const std::uniform_real_distribution<> real_distribution{};

			for (int i = 0; i < LENGTH; ++i)
			{
				auto roll = real_distribution(rnd);
				data_[i] = roll < CROSSOVER_BALANCER
					? p1.data_[i]
					: p2.data_[i];
			}
		}
	}

	double Chromosome::calculate_fitness()
	{
		if (fitness_cache_ <= 0) return fitness_cache_;

		double q_time = 0;

		for (auto i = 0; i < LENGTH; ++i)
		{
			auto& host = get_host(data_[i]);
			auto& job_info = job_table[i];
			if (host.slots_remaining >= job_info.slots())
				host.allocate_immediately(Job(job_table[i]), host_table[data_[i]].cpu_factor);
			else
			{
				while (true)
				{
					auto alloc = host.allocated_jobs.back();
					host.allocated_jobs.pop_back();
					//host.allocated_jobs.
					host.slots_remaining += alloc.slots();

					if (host.slots_remaining >= job_info.slots())
					{
						const auto delay =
							std::max({
								static_cast<uint32_t>(0),
								alloc.finish_time() - job_info.submit_time()
							});

						q_time += delay / 3600.0;

						host.allocate(Job(job_table[i]), delay, host_table[data_[i]].cpu_factor);
						break;
					}
				}
			}
		}

		fitness_cache_ = - q_time;
		return fitness_cache_;
	}
}


