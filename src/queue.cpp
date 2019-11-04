#include <utility>
#include <sstream>
#include <iostream>
#include "../includes/host.h"
#include "../includes/queue.h"
#include "../includes/job.h"
#include "../includes/limit.h"
#include "../includes/cluster_simulation.h"
#include "../dependencies/spdlog/spdlog.h"

//#include "spdlog/spdlog.h"

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
		current_algorithm = algorithm;
	}


	/**
	 * Adds a job to this queue.
	 */
	void Queue::enqueue(Job&& job)
	{
		const std::string& job_name{ job.get_dedicated_host_name() };

		// TODO
		if (job_name == "-" || job_name.empty())
			return;

		jobs_.push_back(job);

		ClusterSimulation::log(LogLevel::info, 
			"Job #{0} is added to Queue {1}.", job.id, this->name);
	}

	/**
	 * Returns a list of hosts being able to execute the given job.
	 */
	std::vector<Host*> Queue::match(const Job& job)
	{	
		// Currently cluster == candHostGroupList
		// To-Do : get_cluster --> Queue.get_host_group_list
		Cluster& cand_host_list{ simulation_->get_cluster() };

		// TODO: Reuse host vector instead of allocating memory each time.
		std::vector<Host*> eligible_host_list{};

		for (auto & [name, host] : cand_host_list)
		{	
			if(host.is_executable(job))
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

				eligible_host_list.push_back(&host);
			}
		}

		return eligible_host_list;
	}

	/**
	 * Sorts eligible hosts.
	 */
	void Queue::sort(std::vector<Host*>::iterator first, std::vector<Host*>::iterator end, const Job& job) const
	{
		//for (auto i{ first }; i != end; ++i)
		//	(*i)->set_rand_score();

		//using namespace std::placeholders;
		//auto f = std::bind(compare_host_function_, _1, _2, job);
		//std::sort(first, end, f);
	}

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
		// 1. Bring back all pending jobs.
		clean_pending_jobs();

		// 2. Sort all jobs using policy.
		policy();

		
		for (auto it = jobs_.begin(); it != jobs_.end(); ++it)
		{
			if (it->is_multi_host)
				it = jobs_.erase(it);
		}

		if (current_algorithm == nullptr)
			for (auto it = jobs_.begin(); it != jobs_.end(); ++it)
				simulation_->
					get_cluster().find_node(it->get_dedicated_host_name())
					->second.execute_job(*it);
		else
		{
			current_algorithm->run(jobs_);
			while (!jobs_.empty())
			{
				Job& job{ jobs_.back() };
				if (job.state != JobState::RUN)
				{
					job.set_pending(simulation_->get_current_time());
					pending_jobs_.push_back(job);
					ClusterSimulation::log(LogLevel::info, "Job #{0} is pended. (pending duration: {1}ms)"
						,job.id, job.total_pending_duration.count());
				}

				jobs_.pop_back();
			}
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
		while(!pending_jobs_.empty())
		{
			Job& pending_job = pending_jobs_.back();
			pending_job.update_total_pending_duration(simulation_->get_current_time());
			if (pending_job.total_pending_duration < std::chrono::microseconds(1000000))
				jobs_.push_back(pending_job);
			pending_jobs_.pop_back();
		}
	}

	// int Queue::using_job_slots() const noexcept
	// {
	// 	int sum{0};
	// 	for (const auto& item : dispatched_hosts_)
	// 		sum += item.second.slot_dispatched;
	// 	return sum;
	// }


	int Queue::id_gen_ = 0;
}

