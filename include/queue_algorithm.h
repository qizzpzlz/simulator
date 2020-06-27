//#pragma once
//#include <utility>
//#include <functional>
//#include <algorithm>
//#include "queue.h"
//#include "host.h"
//
//namespace ClusterSimulator
//{
//	// TODO: As Template
//	class QueueAlgorithm
//	{
//	public:
//		using HostComparer = std::function<bool(const Host*, const Host*, const Job&)>;
//		using HostSorter = std::function<void(
//					std::vector<Host*>&, 
//					const std::vector<Job>&)>;
//		using JobComparer = std::function<bool(const Job&, const Job&)>;
//
//		HostComparer get_host_comparer() const noexcept { return host_comparer_; }
//		HostSorter get_host_sorter() const noexcept { return host_sorter_; }
//		JobComparer get_job_comparer() const noexcept { return job_comparer_; }
//		virtual const std::string& get_name() const noexcept = 0;
//
//	protected:
//		QueueAlgorithm(HostComparer&& host_comparer, JobComparer&& job_comparer)
//			: host_comparer_{host_comparer}, job_comparer_{job_comparer} {}
//
//		explicit QueueAlgorithm(HostComparer&& host_comparer) 
//			: host_comparer_{host_comparer}
//			, job_comparer_{[](const Job&, const Job&) { return false; }}
//			{}
//
//		explicit QueueAlgorithm(HostSorter&& host_sorter)
//			: host_sorter_{host_sorter}
//			, job_comparer_{[](const Job&, const Job&) { return false; }}
//			{}
//
//		HostComparer host_comparer_;
//		HostSorter host_sorter_;
//		JobComparer job_comparer_;
//	};
//
//	/**
//	 * Implementation of OLB Algorithm
//	 */
//	class OLBAlgorithm : public QueueAlgorithm
//	{
//		inline static const std::string name{"OLB"};
//	public:
//		OLBAlgorithm() : QueueAlgorithm(
//			[](const Host* a, const Host* b, const Job&)
//			{
//				return a->remaining_slots() < b->remaining_slots();
//			}) {}
//		const std::string& get_name() const noexcept override { return name; }
//	};
//
//	/**
//	 * Implementation of Minimum Completion Time Algorithm
//	 */
//	class MCTAlgorithm: public QueueAlgorithm
//	{
//		static inline const std::string name{"MCT"};
//
//	public:
//		MCTAlgorithm() : QueueAlgorithm(
//			[](const Host* a, const Host* b, const Job& job)
//			{
//				return get_completion_time(*a, job) < get_completion_time(*b, job);
//			}) {}
//
//		const std::string& get_name() const noexcept override { return name; }
//		static ms get_completion_time(const Host& host, const Job& job) { return host.get_expected_time_of_all_completion() + host.get_expected_run_duration(job); }
//	};
//
//	class MinMinAlgorithm: public QueueAlgorithm
//	{
//		inline static const std::string name{"MinMin"};
//	public:
//		MinMinAlgorithm() : QueueAlgorithm(
//			[](std::vector<Host*>& hosts, const std::vector<Job>& jobs)
//			{
//				// M jobs, N hosts
//				// Prepare a matrix of M * N
//				std::vector< std::vector<ms> > times(jobs.size());
//				for (size_t i = 0; i < times.size(); i++)
//					times.emplace_back(std::vector<ms>(hosts.size()));
//				
//				for (size_t i = 0; i < jobs.size(); i++)
//				{
//					for (size_t j = 0; j < hosts.size(); j++)
//					{
//						times[i][j] = MCTAlgorithm::get_completion_time(*hosts[j], jobs[i]);  // Ready time omitted
//					}
//				}
//
//				std::vector<std::tuple<ms, Host*>> min_completion_times_for_jobs(jobs.size());
//
//				// Find the minimum completion time of task and the host that obtains it
//				for (size_t i = 0; i < jobs.size(); i++)
//				{
//					auto min_iter = std::min_element(times[i].cbegin(), times[i].cend());
//					size_t index = min_iter - times[i].begin();
//					min_completion_times_for_jobs.emplace_back(std::make_tuple(*min_iter, hosts[index]));
//				}
//
//				/*std::sort(min_completion_times_for_jobs.begin(), min_completion_times_for_jobs.end(), 
//					[](std::tuple<ms, Host*>& a, std::tuple<ms, Host*>& b){ return std::get<0>(a) > std::get<0>(b); });*/
//
//				// Find the job with earliest completion time
//				auto min_iter = std::min_element(min_completion_times_for_jobs.cbegin(), min_completion_times_for_jobs.cend());
//				//const Job& min_job = jobs[min_iter - min_completion_times_for_jobs.cbegin()];
//
//				// We don't assign the job to the corresponding host here.
//				// Instead we sort the hosts
//				
//
//			}) {}
//
//			const std::string& get_name() const noexcept override { return name; }
//	};
//
//	class QueueAlgorithms
//	{
//	public:
//		inline static const QueueAlgorithm* const OLB = new OLBAlgorithm();
//		inline static const QueueAlgorithm* const MCT = new MCTAlgorithm();
//		inline static const QueueAlgorithm* const MinMin = new MinMinAlgorithm();
//	};
//}
//
//
//
#pragma once
#include <utility>
#include <functional>
#include <algorithm>
#include "queue.h"
#include "host.h"
#include "cluster.h"
#include "utils.h"
#include <array>

namespace cs
{
	class Cluster;
	// TODO: As Template
	class QueueAlgorithm
	{
	protected:
		using Jobs = std::vector<JobWrapper>;
		static constexpr bool USE_OMP = true;
	public:
		virtual const std::string& get_name() const noexcept = 0;
		virtual void run(Jobs& jobs, Cluster& cluster) const = 0;
	};

	/**
	 * Implementation of Opportunistic Load Balancing Algorithm.
	 */
	class OLBAlgorithm : public QueueAlgorithm
	{
		inline static const std::string name{ "OLB" };
	public:
		const std::string& get_name() const noexcept override { return name; }

		void run(Jobs& jobs, Cluster& cluster) const override
		{
			for (auto& job : jobs)
			{
				auto hosts = job->get_eligible_hosts();
				if (hosts.empty()) continue;
				auto best_host = Utils::min_element_parallel(hosts,
					[](const Host* a, const Host* b)
					{
						return a->remaining_slots() > b->remaining_slots();
					}).second;
				/*best_host->execute_job(std::move(job));*/
				job.execute(best_host);
			}
		}
	};

	/**
	 * Implementation of Minimum Completion Time Algorithm.
	 */
	class MCTAlgorithm : public QueueAlgorithm
	{
		inline static const std::string name{ "MCT" };
		static ms get_completion_time(const Host& host, const Job& job)
		{
			return host.get_expected_time_of_all_completion() + host.get_expected_run_duration(job);
		}
	public:
		const std::string& get_name() const noexcept override { return name; }

		void run(Jobs& jobs, Cluster& cluster) const override
		{
			for (auto& job : jobs)
			{
				auto hosts = job->get_eligible_hosts();
				if (hosts.empty()) continue;
				auto best_host = Utils::min_element_parallel(hosts,
					[&job](const Host* a, const Host* b)
					{
						/*return get_completion_time(*a, *job) < get_completion_time(*b, *job);*/
						return a->get_expected_completion_duration(*job) < b->get_expected_completion_duration(*job);
					}).second;
				auto ready_duration = best_host->get_ready_duration(*job);
				if (ready_duration > 0ms)
					job.execute_when_ready(best_host, ready_duration);
				else
					job.execute(best_host);
			}
		}
	};

	/**
	 * Implementation of Min-Min Algorithm.
	 */
	class MinMinAlgorithm : public QueueAlgorithm
	{
		inline static const std::string name{ "MinMin" };
		const std::string& get_name() const noexcept override { return name; }

		//Utils::PoolAllocator pool_allocator;

		struct JobProxy
		{
			//JobWrapper* job;
			size_t job_index;
			milliseconds min_mct;
			size_t host_index_of_min_mct;
			std::vector<milliseconds> mcts;
			//milliseconds* mcts;

			//JobProxy();
			JobProxy(size_t job_index, size_t size) : job_index{ job_index }, mcts(size)
			{
				//mcts = pool_allocator(0ms);
				//mcts.reserve(size);
			}
			void Update(const std::pair<milliseconds, size_t>& pair)
			{
				min_mct = pair.first;
				host_index_of_min_mct = pair.second;
			}
		};
public:
		//MinMinAlgorithm(size_t cluster_size) : pool_allocator(cluster_size) {}

		void run(Jobs& jobs, Cluster& cluster) const override
		{
			// Prepare space for Hosts having minimum completion time for each job.
			// Prepare a matrix of M * N for M jobs and N hosts.
			std::vector<JobProxy> all_jobs;
			all_jobs.reserve(jobs.size());

			// Possible parallelization.
			int i;
			for (i = 0; i < jobs.size(); i++)
			{
				// Find the MCT host together in this loops.
				JobProxy proxy(i, cluster.count());
				std::pair<milliseconds, size_t> min_pair = std::make_pair(milliseconds::max(), 0);
				
				if constexpr (config::USE_STATIC_HOST_TABLE_FOR_JOBS)
				{
					auto& compatible_host_ids = jobs[i]->get_compatible_hosts();
					for (auto& ms : proxy.mcts)
						ms = milliseconds::max();
					std::size_t size = compatible_host_ids.size();
					long long j;
//					if constexpr (USE_OMP)
//#pragma omp parallel for
					for (j = 0; j < size; ++j)
					{
						auto id = compatible_host_ids[j];
						auto& host = cluster[id];
						milliseconds completion_time = host.get_expected_completion_duration(*jobs[i]);
						proxy.mcts[id] = completion_time;
						if (completion_time < std::get<0>(min_pair))
							min_pair = std::make_pair(completion_time, id);
					}
				}
				else
				{
					int j;
					if constexpr (USE_OMP)
#pragma omp parallel for
						for (j = 0; j < cluster.count(); j++)
						{
							auto completion_time = cluster[j].get_expected_completion_duration(*jobs[i]);
							proxy.mcts[j] = completion_time;
							if (completion_time < std::get<0>(min_pair))
								min_pair = std::make_pair(completion_time, j);
						}
				}

				proxy.Update(min_pair);
				all_jobs.push_back(std::move(proxy));
			}

			// For each remaining jobs
			do
			{
				// Find the minimum completion time
				const auto min_iter = std::min_element(all_jobs.cbegin(), all_jobs.cend(),
					[](const JobProxy& a, const JobProxy& b)
					{
						return a.min_mct < b.min_mct;
					});
				const auto assigned_host_id = min_iter->host_index_of_min_mct;
				const auto assigned_host = &cluster[assigned_host_id];

				// Assign the job with the minimum mct to the dedicated host.
				if (min_iter->min_mct != milliseconds::max()) // Skip the incompatible jobs
				{
					auto& job = jobs[min_iter->job_index];
					const auto ready_duration = assigned_host->get_ready_duration(*job);
					if (ready_duration > 0ms)
						// Can't be executed right now;
						job.execute_when_ready(assigned_host, ready_duration);
					else
						job.execute(assigned_host);
				}

				all_jobs.erase(min_iter);

				// Update completion time for the assigned host
				// and find the minimum mct for each job 
				for (auto& proxy : all_jobs)
				{
					milliseconds updated_completion_time;
					if constexpr (config::USE_STATIC_HOST_TABLE_FOR_JOBS)
					{
						if (proxy.mcts.at(assigned_host_id) != milliseconds::max())
						{	// The assigned host is compatible for this job
							updated_completion_time = assigned_host->get_expected_completion_duration(*jobs[proxy.job_index]);
						}
						else
						{
							// Don't need to consider jobs that are not compatible with the assigned host.
							continue;
						}
					}
					else
						updated_completion_time =
							assigned_host->get_expected_completion_duration(*jobs[proxy.job_index]);

					proxy.mcts[assigned_host_id] = updated_completion_time;
					
					if (proxy.host_index_of_min_mct == assigned_host_id)
					{	// Only updates the minimum mct if its dedicated host is changed.
						if (updated_completion_time > proxy.min_mct)
						{
							auto min_iter_inner = std::min_element(proxy.mcts.cbegin(), proxy.mcts.cend());
							proxy.min_mct = *min_iter_inner;
							proxy.host_index_of_min_mct = std::distance(proxy.mcts.cbegin(), min_iter_inner);
						}
					}
				}
			} while (!all_jobs.empty());
		}
	};

	class WeightBasedGreedySelection : public QueueAlgorithm
	{
		inline static std::string name_ = "Weight Based Greedy Selection";
	public:
		static constexpr unsigned num_weights = 5;
		std::array<double, num_weights> weights;

		auto factor_w() const noexcept { return weights[0]; }
		auto max_slots_w() const noexcept { return weights[1]; }
		auto remaining_slots_w() const noexcept { return weights[2]; }
		auto expected_remaining_slots_w() const noexcept { return weights[3]; }
		auto ready_duration_w() const noexcept { return weights[4]; }
		
		explicit WeightBasedGreedySelection(std::array<double, num_weights> weights)
		: weights{std::move(weights)} { }
		
		const std::string& get_name() const noexcept override { return name_; }
		void run(Jobs& jobs, Cluster& cluster) const override
		{
			static std::vector<double> scores(cluster.count());
			
			for (auto& job : jobs)
			{
				for (int i = 0; i < cluster.count(); ++i)
				{
					auto& host = cluster[i];
					if (!host.is_compatible(*job))
					{
						scores[i] = 0;
						continue;
					}
					
					double score = 0;
					score += host.cpu_factor * factor_w();
					score += host.max_slot * max_slots_w();
					score += host.remaining_slots() * remaining_slots_w();

					const auto expected_remaining_slots = host.get_expected_remaining_slots(*job);
					score += expected_remaining_slots * expected_remaining_slots_w();
					seconds ready_duration_seconds =
						duration_cast<seconds>(host.get_ready_duration(*job));
					score += ready_duration_seconds.count() * ready_duration_w();

					scores[i] = score;
				}

				auto it  = std::max_element(scores.cbegin(), scores.cend());

				job.execute(&cluster[std::distance(scores.cbegin(), it)]);
			}
		}
	};
	
	class QueueAlgorithms
	{
	public:
		inline static const QueueAlgorithm* const OLB = new OLBAlgorithm();
		inline static const QueueAlgorithm* const MCT = new MCTAlgorithm();
		inline static const QueueAlgorithm* const MinMin = new MinMinAlgorithm();
		//inline static const QueueAlgorithm* const WeightBased = new WeightBasedGreedySelection();
	};
}



