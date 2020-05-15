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
			std::uniform_int_distribution<> dist(0, job_table[i].num_eligible_hosts() - 1);
			data_[i] = job_table[i].get_host(dist(rnd_));
		}
	}

	Chromosome Chromosome::mutate() const
	{
		std::uniform_real_distribution<> real_distribution{};

		Chromosome child{};
		child.type_ = Type::MUTATION;
		std::copy(data_.begin(), data_.end(), child.data_.begin());
		for (int i = 0; i < LENGTH; ++i)
		{
			// auto roll = real_distribution(rnd_);
			auto roll = real_distribution(rnd_);
			if (roll >= GENE_MUTATION_PROBABILITY) continue;

			std::uniform_int_distribution<> dist(0, job_table[i].num_eligible_hosts() - 1);
			child.data_[i] = job_table[i].get_host(dist(rnd_));
		}

		return child;
	}

	Chromosome Chromosome::crossover(const Chromosome& other) const
	{
		Chromosome child{};
		child.type_ = Type::CROSSOVER;


		if constexpr (CROSSOVER_TYPE == CrossoverTypes::Uniform)
		{
			std::uniform_real_distribution<> real_distribution{};

			for (int i = 0; i < LENGTH; ++i)
			{
				auto roll = real_distribution(rnd_);
				child.data_[i] = roll < CROSSOVER_BALANCER
					? data_[i]
					: other.data_[i];
			}
		}
		else if constexpr (CROSSOVER_TYPE == CrossoverTypes::SinglePoint)
		{
			std::uniform_int_distribution<> int_distribution(0, LENGTH - 1);
			auto point = int_distribution(rnd_);
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
			std::uniform_real_distribution<double> real_distribution{};

			for (int i = 0; i < LENGTH; ++i)
			{
				auto roll = real_distribution(rnd_);
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
		auto hosts = host_prototypes;

		for (auto i = 0; i < LENGTH; ++i)
		{
			auto& host = hosts[data_[i]];
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
						uint32_t delay;

						if(alloc.finish_time() > job_info.submit_time())
							delay = alloc.finish_time() - job_info.submit_time();
						else
							delay = 0;

						q_time += delay / 3600.0;

						host.allocate(Job(job_table[i]), delay, host_table[data_[i]].cpu_factor);
						break;
					}
				}
				//
				//for (auto it = host.allocated_jobs.rbegin(); it != host.allocated_jobs.rend(); ++it)
				//{
				//
				//}
			}
		}

		fitness_cache_ = - q_time;
		return fitness_cache_;
	}
}


