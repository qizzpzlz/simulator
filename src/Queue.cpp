#include <utility>
#include <sstream>
#include <iostream>
#include "../includes/Host.h"
#include "../includes/Queue.h"
#include "../includes/Job.h"
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
	{
		is_default_ = true;
	}
		

	Queue::Queue(ClusterSimulation& simulation, int priority, std::string name) 
		: name{ std::move(name) }
		, priority{ priority }
		, simulation_{ &simulation } 
	{ 

	}

	Queue::Queue(ClusterSimulation& simulation, const std::string& name) 
		: name{name}
		, priority{StaticQueueData::queue_priorities[name]}
		, simulation_{ &simulation }
	{ 

	}

	Queue::~Queue()
	= default;

	void Queue::enqueue(Job&& job)
	{
		const std::string& job_name = job.get_dedicated_host_name();

		// TODO
		if (job_name == "-" || job_name.empty())
			return;

		jobs_.push_back(job);

		ClusterSimulation::log(LogLevel::info, 
			"Job #{0} is added to Queue {1}.", job.id, this->name);
	}

	//Doing filtering on candidate hosts
	Queue::Host_List Queue::match(const Job& job)
	{	
		//지금은 cluster = candHostGroupList
		//To-Do : get_cluster --> Queue.get_host_group_list
		Cluster& cand_host_list{simulation_->get_cluster()};
		Host_List eligible_host_list;

		for (Host& host : cand_host_list)
		{	
			if(host.is_executable(job))
			{
				eligible_host_list.push_back(std::reference_wrapper<Host>(host));
			}
		}

		return eligible_host_list;
	}

	Queue::Host_List Queue::sort()
	{
		while (!jobs_.empty())
		{
			//const Job& job{jobs_.top()};
			Job& job{jobs_.back()};
			auto sorted_host_list = match(job);
			
			//찾지 못했을때
			if (sorted_host_list.empty())
			{
				auto dedicated_host{ simulation_->find_host(job.get_dedicated_host_name()) };
				//PEND
				job.set_pending(simulation_->get_current_time());
				//throw std::out_of_range("Can't find a host to dispatch");
				pending_jobs_.push_back(job);
				ClusterSimulation::log(LogLevel::info, "Job #{0} is pended. (pend start time: {2}ms)"
							, job.id, job.run_time.count());

				ClusterSimulation::log_jobmart(job);
				continue;
			}
			
			std::sort(sorted_host_list.begin(), sorted_host_list.end(), 
			 compare_host_function);
			//std::cout << sorted_host_list.back().get().score() << " ";
			return sorted_host_list;
		}
	}

	//job의 순서를 정함
	void Queue::policy()
	{
		//Scheduling policy
		//jobs_.push();
		clean_pending_jobs();

		//std::sort(jobs_.begin(), jobs_.end(), compare_job_function);
	}

	bool Queue::dispatch()
	{
		bool flag{ false };

		policy();

		// For each job in the current queue
		// find the best host and dispatch to it or
		// delay it.
		while (!jobs_.empty())
		{
			flag = true;

			// Pop a job from the inner queue;
			Job& job{jobs_.back()};

			// TODO: Multi-host jobs
			if (job.is_multi_host)
			{
				jobs_.pop_back();
				continue;
			}
			
			auto eligible_hosts = sort();
			
			if(eligible_hosts.empty())
			{	
				//throw std::out_of_range("Can't find a host to dispatch");
			
				// Delay the job.
				job.set_pending(simulation_->get_current_time());
				pending_jobs_.push_back(job);
				jobs_.pop_back();
				ClusterSimulation::log(LogLevel::info, "Job #{0} is pended. (pend start time: {2}ms)"
							, job.id, job.run_time.count());

				ClusterSimulation::log_jobmart(job);
				continue;
			}

			// Find best available host
			while (!eligible_hosts.empty())
			{
				Host& best_host = eligible_hosts.back();

				//check if best_host is executable
				//if(best_host.is_executable(job)) --> Why would we check availability here again?
				{
					ClusterSimulation::log(LogLevel::info, "Queue {0} dispatches Job #{1} to Host {2}"
						,name, job.id, best_host.name);
					
					//TO-DO : host.register()
					best_host.execute_job(job);
					
					// TODO: Host.set_start_time()
					const auto start_time = simulation_->get_current_time();
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
					break;
				}
				// else
				// {
				// 	eligible_hosts.pop();
				// 	continue;
				// }
			}
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

	int Queue::id_gen_ = 0;
}

