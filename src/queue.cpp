#include <utility>
#include <sstream>
#include <iostream>
#include "../includes/host.h"
#include "../includes/queue.h"
#include "../includes/job.h"
#include "../includes/limit.h"
#include "../includes/cluster_simulation.h"
#include "../dependencies/spdlog/spdlog.h"

#include <omp.h>

namespace ClusterSimulator
{
	std::map<std::string, int> Queue::StaticQueueData::queue_priorities{};
	const Queue::StaticQueueData Queue::data{};

	Queue::Queue(ClusterSimulation& simulation) 
		: name{ "normal" }
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

	void Queue::set_algorithm(const QueueAlgorithm* const algorithm) noexcept
	{
		if (algorithm == nullptr)
			throw std::runtime_error("Cannot set null algorithm.");
		
		current_algorithm = algorithm;
	}


	/**
	 * Adds a job to this queue.
	 */
	void Queue::enqueue(Job&& job)
	{
		jobs_.push_back(std::make_shared<Job>(job));

		ClusterSimulation::log(LogLevel::info, 
			"Job #{0} is added to Queue {1}.", job.id, this->name);
	}

	/**
	 * Returns a list of hosts being able to execute the given job.
	 */
	std::vector<Host*> Queue::match(const Job& job)
	{	
		std::vector<Host>& cand_host_list{ simulation_->get_cluster().vector() };

		// TODO: Reuse host vector instead of allocating memory each time.
		std::vector<Host*> eligible_host_list{};

		int max = omp_get_max_threads();
		std::vector <std::vector<Host* >> hosts(max);

		int i;
		#pragma omp parallel for
		for (i = 0; i < cand_host_list.size(); ++i)
		{
			auto tid = omp_get_thread_num();

			Host& host = cand_host_list[i];
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

				//eligible_host_list.push_back(&host);
				hosts[tid].push_back(&host);
			}
		}

		for (auto& vec : hosts)
			eligible_host_list.insert(eligible_host_list.end(), vec.begin(), vec.end());

		return eligible_host_list;
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

		current_algorithm->run(jobs_);
		while (!jobs_.empty())
		{
			std::shared_ptr<Job>& job{jobs_.back()};

			// Set pending for not assigned jobs
			if (job->state != JobState::RUN)
			{
				job->set_pending(simulation_->get_current_time());
				pending_jobs_.push_back(job);
				ClusterSimulation::log(LogLevel::info, "Job #{0} is pended. (pending duration: {1}ms)"
				                       ,job->id, job->total_pending_duration.count());
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
		for (auto& job : pending_jobs_)
		{
			job->update_total_pending_duration(simulation_->get_current_time());
			if (job->total_pending_duration < std::chrono::hours(1))
				jobs_.push_back(job);
			else
			{
				simulation_->increment_failed_jobs();
				ClusterSimulation::log(LogLevel::warn, "Job #{0} is discarded. (exceeds the maximum pending duration. slot_req: {1})"
					,job->id, job->slot_required);
			}
			//jobs_.push_back(job);
		}

		pending_jobs_.clear();
	}

	int Queue::id_gen_ = 0;
}

