#include <utility>
#include <sstream>
#include <iostream>
#include "../includes/Host.h"
#include "../includes/Queue.h"
#include "../includes/Job.h"
#include "../includes/Limit.h"
#include "../includes/Cluster_Simulation.h"
#include "../dependencies/spdlog/spdlog.h"

//#include "spdlog/spdlog.h"

namespace ClusterSimulator
{
	std::map<std::string, int> Queue::StaticQueueData::queue_priorities{};
	const Queue::StaticQueueData Queue::data{};

	Queue::Queue(ClusterSimulation& simulation) 
		: name{ "normal" }
		, priority{ DEFAULT_PRIORITY }
		, simulation_{ &simulation }
		, is_default_{ true } {}
		
	Queue::Queue(ClusterSimulation& simulation, int priority, std::string name) 
		: name{ std::move(name) }
		, priority{ priority }
		, simulation_{ &simulation } {}

	Queue::Queue(ClusterSimulation& simulation, const std::string& name) 
		: name{name}
		, priority{StaticQueueData::queue_priorities[name]}
		, simulation_{ &simulation } {}

	Queue::~Queue() = default;


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
	 * Returns a list of hosts ables to execute the given job.
	 */
	Queue::HostList Queue::match(const Job& job)
	{	
		// Currently cluster == candHostGroupList
		// To-Do : get_cluster --> Queue.get_host_group_list
		Cluster& cand_host_list{ simulation_->get_cluster() };

		// TODO: Reuse host vector instead of allocating memory each time.
		HostList eligible_host_list{};

		for (Host& host : cand_host_list)
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
	void Queue::sort(Queue::HostList::iterator first, Queue::HostList::iterator end)
	{
		for (auto i{ first }; i != end; i++)
			(*i)->set_rand_score();

		std::sort(first, end, compare_host_function_);
	}

	/**
	 * Determines dispatch order of jobs in this queue.
	 * Currently there is no explicit policy.
	 */
	void Queue::policy()
	{	
		// Retrive all pending jobs to the primary job list.
		clean_pending_jobs();

		// TODO: policy
		//std::sort(jobs_.begin(), jobs_.end(), compare_job_function);
	}

	/**
	 * Dispatchs all jobs in this queue.
	 * Jobs that couldn't be dispatched this time are
	 * pending until the next dispatch.
	 */
	bool Queue::dispatch()
	{
		// Returns true if any jobs are dispatched this time.
		bool flag{ false };

		// 1. Bring back all pending jobs.
		// 2. Sort all jobs using policy.
		policy();

		// For each job in the current queue
		// find the best host and dispatch to it or
		// delay it.
		while (!jobs_.empty())
		{
			// Peek a job from the inner queue;
			Job& job{ jobs_.back() };

			// TODO: Multi-host jobs
			if (job.is_multi_host)
			{
				jobs_.pop_back();
				continue;
			}

			// 
			if( qjob_limit != -1 && using_job_slots() > qjob_limit )
			{
				ClusterSimulation::log(LogLevel::info, "Reached qjob_limit");
				job.set_pending(simulation_->get_current_time());
				pending_jobs_.push_back(job);
				ClusterSimulation::log(LogLevel::info, "Job #{0} is pended. (pend start time: {1}ms)"
								, job.id, job.run_time.count());
				

				jobs_.pop_back();
				continue;
			}

			// Get all avilable hosts for the current job.
			Queue::HostList eligible_hosts{ match(job) };
			
			// If there is no eligible host for this job,
			// pend the job.
			if (eligible_hosts.empty())
			{
				ClusterSimulation::log(LogLevel::info, "Job #{0} is pended. (pend start time: {1}ms)"
								, job.id, job.run_time.count());
				job.set_pending(simulation_->get_current_time());
				pending_jobs_.push_back(job);

				jobs_.pop_back();
				
				continue;
			}

			// flag = true;
			
			// TODO: Why would we sort hosts? 
			// Can we just get the host with the maximum priority?
			sort(eligible_hosts.begin(), eligible_hosts.end());

			// Find best available host
			Host& best_host{ *eligible_hosts.back() };

			if(is_exclusive)
			{
				ClusterSimulation::log(LogLevel::info, "Dispatch job #{0} in exclusive queue {1}"
							, job.id, name);

				if(best_host.num_current_jobs !=0)
				{
					ClusterSimulation::log(LogLevel::info, "Cannot dispatch job #{0}. (Host {1} has running jobs)"
								, job.id, best_host.name);
					
					ClusterSimulation::log(LogLevel::info, "Job #{0} is pended. (pend start time: {1}ms)"
								, job.id, job.run_time.count());
					job.set_pending(simulation_->get_current_time());
					pending_jobs_.push_back(job);

					jobs_.pop_back();
					continue;
				}
			}
			
			//limits
			//if(cpu_limit != -1 && total_cpu_time > cpu_limit)
			//{
				//CPU_LIMIT ..?? 
			
			//qjob_limit
			
			// Register the best host to the dispatched hosts list.
			auto search = dispatched_hosts_.find(&best_host);
			if (search != dispatched_hosts_.end())
				dispatched_hosts_.insert(std::make_pair(&best_host, HostInfo{job.slot_required}));
			else
			{
				dispatched_hosts_[&best_host].slot_dispatched += job.slot_required;
			}

			flag = true;
			
			ClusterSimulation::log(LogLevel::info, "Queue {0} dispatches Job #{1} to Host {2}"
				,name, job.id, best_host.name);

			// TO-DO : host.register()
			best_host.execute_job(job);
			
			// TODO: Host.set_start_time()
			const ms start_time = simulation_->get_current_time();
			job.start_time = start_time;

			simulation_->after_delay(job.run_time, [&best_host, job]
			{
				best_host.exit_job(job);

				std::stringstream ss(job.get_exit_host_status());
				ss >> Utils::enum_from_string<HostStatus>(best_host.status);
				ClusterSimulation::log(LogLevel::info, "Job #{0} is finished in Host {1}. (run time: {2}ms)"
					, job.id, best_host.name, job.run_time.count());

				ClusterSimulation::log_jobmart(job);
			});

			jobs_.pop_back();
		}
		// Returns true if there exists any pending jobs
		return flag;
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
			jobs_.push_back(pending_job);
			pending_jobs_.pop_back();
		}
	}

	int Queue::using_job_slots() const noexcept
	{
		int sum{0};
		for (auto& item : dispatched_hosts_)
		{
			sum += item.second.slot_dispatched;
		}
		return sum;
	}

	int Queue::id_gen_ = 0;
}

