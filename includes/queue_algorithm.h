#pragma once
#include <utility>
#include <functional>
#include <algorithm>
#include "queue.h"
#include "host.h"

namespace ClusterSimulator
{
	// TODO: As Template
	class QueueAlgorithm
	{
	public:
		using HostComparer = std::function<bool(const Host*, const Host*, const Job&)>;
		using HostSorter = std::function<void(
					std::vector<Host*>&, 
					const std::vector<Job>&)>;
		using JobComparer = std::function<bool(const Job&, const Job&)>;

		HostComparer get_host_comparer() const noexcept { return host_comparer_; }
		HostSorter get_host_sorter() const noexcept { return host_sorter_; }
		JobComparer get_job_comparer() const noexcept { return job_comparer_; }

	protected:
		QueueAlgorithm(HostComparer&& host_comparer, JobComparer&& job_comparer)
			: host_comparer_{host_comparer}, job_comparer_{job_comparer} {}

		explicit QueueAlgorithm(HostComparer&& host_comparer) 
			: host_comparer_{host_comparer}
			, job_comparer_{[](const Job& a, const Job& b) { return false; }}
			{}

		explicit QueueAlgorithm(HostSorter&& host_sorter)
			: host_sorter_{host_sorter}
			, job_comparer_{[](const Job& a, const Job& b) { return false; }}
			{}

		HostComparer host_comparer_;
		HostSorter host_sorter_;
		JobComparer job_comparer_;
	};

	/**
	 * Implementation of OLB Algorithm
	 */
	class OLBAlgorithm : public QueueAlgorithm
	{
	public:
		OLBAlgorithm() : QueueAlgorithm(
			[](const Host* a, const Host* b, const Job&)
			{
				return a->remaining_slots() < b->remaining_slots();
			}) {}
	};

	/**
	 * Implementation of Minimum Completion Time Algorithm
	 */
	class MCTAlgorithm: public QueueAlgorithm
	{
	public:
		MCTAlgorithm() : QueueAlgorithm(
			[](const Host* a, const Host* b, const Job& job)
			{
				return get_completion_time(*a, job) < get_completion_time(*b, job);
			}) {}

		static ms get_completion_time(const Host& host, const Job& job) { return host.get_expected_time_of_all_completion() + host.get_expected_run_time(job); }
	};

	class MinMinAlgorithm: public QueueAlgorithm
	{
	public:
		MinMinAlgorithm() : QueueAlgorithm(
			[](std::vector<Host*>& hosts, const std::vector<Job>& jobs)
			{
				// M jobs, N hosts
				// Prepare a matrix of M * N
				std::vector< std::vector<ms> > times(jobs.size());
				for (size_t i = 0; i < times.size(); i++)
					times.emplace_back(std::vector<ms>(hosts.size()));
				
				for (size_t i = 0; i < jobs.size(); i++)
				{
					for (size_t j = 0; j < hosts.size(); j++)
					{
						times[i][j] = MCTAlgorithm::get_completion_time(*hosts[j], jobs[i]);  // Ready time omitted
					}
				}

				std::vector<std::tuple<ms, Host*>> min_completion_times_for_jobs(jobs.size());

				// Find the minimum completion time of task and the host that obtains it
				for (size_t i = 0; i < jobs.size(); i++)
				{
					auto min_iter = std::min_element(times[i].cbegin(), times[i].cend());
					size_t index = min_iter - times[i].begin();
					min_completion_times_for_jobs.emplace_back(std::make_tuple(*min_iter, hosts[index]));
				}

				/*std::sort(min_completion_times_for_jobs.begin(), min_completion_times_for_jobs.end(), 
					[](std::tuple<ms, Host*>& a, std::tuple<ms, Host*>& b){ return std::get<0>(a) > std::get<0>(b); });*/

				// Find the job with earliest completion time
				auto min_iter = std::min_element(min_completion_times_for_jobs.cbegin(), min_completion_times_for_jobs.cend());
				const Job& min_job = jobs[min_iter - min_completion_times_for_jobs.cbegin()];

				// We don't assign the job to the corresponding host here.
				// Instead we sort the hosts
				

			}) {}
	};

	class QueueAlgorithms
	{
	public:
		inline static const QueueAlgorithm* const OLB = new OLBAlgorithm();
		inline static const QueueAlgorithm* const MCT = new MCTAlgorithm();
		inline static const QueueAlgorithm* const MinMin = new MinMinAlgorithm();
	};

	inline extern QueueAlgorithms QAlgorithms{};

}



