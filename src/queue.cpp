#include "host.h"
#include "queue.h"
#include "job.h"
#include "limit.h"
#include "cluster_simulation.h"
#include "queue_algorithm.h"
#include <utility>
#include <omp.h>

//#include "spdlog/spdlog.h"

namespace cs
{
	std::map<std::string, int> Queue::StaticQueueData::queue_priorities{};
	const Queue::StaticQueueData Queue::data{};

	Queue::Queue(ClusterSimulation& simulation) 
		: name{ "default" }
		, priority{ DEFAULT_PRIORITY }
		, is_default_{ true }
		, simulation_{ &simulation } {}
		
	Queue::Queue(ClusterSimulation& simulation, int priority, std::string name) 
		: name{ std::move(name) }
		, priority{ priority }
		, simulation_{ &simulation } {}

	Queue::Queue(ClusterSimulation& simulation, const std::string& name) 
		: name{name}
		, priority{StaticQueueData::queue_priorities[name]}
		, simulation_{ &simulation } {}

	Queue::~Queue() = default;

	void Queue::set_algorithm(const QueueAlgorithm* const algorithm)
	{
		if (algorithm == nullptr)
			throw std::runtime_error("Cannot set null algorithm.");

		current_algorithm = algorithm;

		//if (algorithm->get_host_comparer())
		//	set_compare_host_function_(algorithm->get_host_comparer());
		//else
		//	set_compare_host_function_(nullptr);
	}


	/**
	 * Adds a job to this queue.
	 */
	void Queue::enqueue(std::shared_ptr<Job> job)
	{
		if constexpr (ClusterSimulation::LOG_ANY)
			simulation_->log(LogLevel::info,
				"Job #{0} is added to Queue {1}.", job->id, this->name);
		jobs_.push_back(std::move(job));
	}

	void Queue::add_pending_job(JobWrapper& job)
	{
		job->set_pending(simulation_->get_current_time());
		if constexpr (ClusterSimulation::LOG_ANY)
			simulation_->log(LogLevel::info, "Job #{0} is pended. (pending duration: {1}ms)"
				, job->id, job->total_pending_duration.count());
		pending_jobs_.push_back(std::move(job));
	}

	void Queue::add_pending_job(JobWrapper&& job)
	{
		pending_jobs_.push_back(job);
		job->set_pending(simulation_->get_current_time());
		if constexpr (ClusterSimulation::LOG_ANY)
			simulation_->log(LogLevel::info, "Job #{0} is pended. (pending duration: {1}ms)"
				, job->id, job->total_pending_duration.count());
	}

	/**
	  * Returns a list of hosts being able to execute the given job.
	  */
	std::vector<Host*> Queue::match(const Job& job)
	{	
		Cluster& cluster{ simulation_->get_cluster() };

		// TODO: Reuse host vector instead of allocating memory each time.
		std::vector<Host*> eligible_host_list{};

		auto& hosts{ get_match_hosts_cache() };

		int i;
#pragma omp parallel for
		for (i = 0; i < cluster.count(); ++i)
		{
			auto tid = omp_get_thread_num();

			Host& host = cluster[i];
			if (host.is_executable(job))
			{
				bool flag{ false };
				for (auto& limit : limits)
				{
					if (!limit->is_eligible(*this, host, job))
					{
						flag = true;
						break;
					}
				}
				if (flag) continue;

				hosts[tid].push_back(&host);
			}
		}

		for (auto& vec : hosts)
		{
			eligible_host_list.insert(eligible_host_list.end(), vec.begin(), vec.end());
			vec.clear();
		}

		return eligible_host_list;
	}

	///**
	// * Sorts eligible hosts.
	// */
	//void Queue::sort(Queue::HostList::iterator first, Queue::HostList::iterator end, const Job& job) const
	//{
	//	//for (auto i{ first }; i != end; ++i)
	//	//	(*i)->set_rand_score();

	//	using namespace std::placeholders;
	//	auto f = std::bind(compare_host_function_, _1, _2, job);
	//	std::sort(first, end, f);
	//}

	/**
	 * Determines dispatch order of jobs in this queue.
	 * Currently there is no explicit policy.
	 */
	void Queue::policy()
	{	
		//if (current_algorithm && current_algorithm->get_job_comparer())
		//	std::sort(jobs_.begin(), jobs_.end(), current_algorithm->get_job_comparer());
	}

	/**
	 * Dispatchs all jobs in this queue.
	 * Jobs that could not be dispatched this time are
	 * pending until the next dispatch.
	 */
	bool Queue::dispatch()
	{
		// Bring back all pending jobs.
		clean_pending_jobs();

		if (jobs_.empty()) return false;

		// Run algorithm.
		current_algorithm->run(jobs_, simulation_->get_cluster());

		// Pend unassigned jobs.
		while (!jobs_.empty())
		{
			/*Job& job{ *jobs_.back() };*/
			JobWrapper& job{ jobs_.back() };

			// Set pending for not assigned jobs
			if (!job.is_dispatched())
			{
				add_pending_job(job);
			}

			jobs_.pop_back();
		}

		// Returns true if there exists any pending jobs
		return !pending_jobs_.empty();
	}

	/*
	 * Move back all pending jobs to the main queue.
	 * Currently total pending time of a job is accumulated to its priority.
	 */
	void Queue::clean_pending_jobs()
	{
		if (pending_jobs_.empty()) return;
		
		for (auto& job : pending_jobs_)
		{
			job->update_total_pending_duration(simulation_->get_current_time());
			if (job->total_pending_duration < hours(1))
				jobs_.push_back(std::move(job));
			else
			{
				simulation_->increment_failed_jobs();
				if constexpr (ClusterSimulation::LOG_ANY)
					simulation_->log(LogLevel::warn, "Job #{0} is discarded. (exceeds the maximum pending duration. slot_req: {1})"
						, job->id, job->slot_required);
			}
		}

		pending_jobs_.clear();
	}

	auto Queue::get_match_hosts_cache() -> decltype(match_hosts_cache_)&
	{
		if (!match_hosts_cache_initialised)
		{
			auto max_threads = omp_get_max_threads();
			auto per_vec_count = simulation_->get_cluster_view().count() / max_threads;
			for (auto i = 0; i < max_threads; ++i)
			{
				std::vector<Host*> vec;
				vec.reserve(per_vec_count);
				match_hosts_cache_.push_back(std::move(vec));
			}

			match_hosts_cache_initialised = true;
		}
		return match_hosts_cache_;
	}

	int Queue::id_gen_ = 0;
}

