#pragma once
#include <array>
#include <vector>
#include "genetic_algorithm.h"
#include <memory_resource>
#include <random>

namespace genetic
{
	struct Entry;

	class Chromosome
	{
	public:
		using const_iterator = std::vector<uint16_t>::const_iterator;

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

		static Chromosome&& create_random()
		{
			static thread_local std::default_random_engine rnd;
			std::uniform_int_distribution<> dist(0, NUM_HOSTS);
			Chromosome chromosome(LENGTH);
			auto& data = chromosome.data_;
			for (int i = 0; i < LENGTH; ++i)
				data[i] = dist(rnd);
			return std::move(chromosome);
		}

		void make_random()
		{
			static thread_local std::default_random_engine rnd;
			//std::uniform_int_distribution<> dist(0, NUM_HOSTS);
			//for (int i = 0; i < LENGTH; ++i)
			//	data_[i] = dist(rnd);
			//
			for (int i = 0; i < LENGTH; ++i)
			{
				auto& eligible_hosts = job_table[i].hosts;
				std::uniform_int_distribution<> dist(0, eligible_hosts.size() - 1);
				data_[i] = eligible_hosts[dist(rnd)].value;
			}
		}

		double calculate_fitness()
		{
			if (fitness_cache >= 0) return fitness_cache;
			
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
								std::max({ static_cast<uint32_t>(0),
									alloc.finish_time() - job_info.submit_time() });

							q_time += delay / 3600;

							host.allocate(Job(job_table[i]), delay, host_table[data_[i]].cpu_factor);
							break;
						}
					}
				}
			}

			fitness_cache = q_time;
			return q_time;
		}
		
	private:
		explicit Chromosome(uint16_t length)
			: data_(length)
			, hosts_(host_prototypes)
		{
			
		}
		std::vector<uint16_t> data_;
		std::vector<Host> hosts_;
		double fitness_cache = -1;
	};
}
