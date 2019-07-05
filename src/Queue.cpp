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

	//Queue::Queue(const Queue& other)
	//	: name{other.name}
	//	, id{other.id}
	//	, priority{other.priority}
	//	, simulation_{other.simulation_}
	//{
	//	std::copy(other.jobs_.)
	//}

	//Queue& Queue::operator=(const Queue&)
	//{
	//}

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

		if (job_name == "-" || job_name.empty())
			return;

		//jobs_.push_back(job);
		jobs_.push(job);
		spdlog::info("Job #{0} is added to Queue {1}.", job.id, this->name);
		ClusterSimulation::file_logger->info("Job #{0} is added to Queue {1}.", job.id, this->name);

		//##################################################

		// TODO: instant dispatch
		/* 
		jobs_.clear();
		spdlog::info("Job #{0} is dispatched to Host {1}", job.id, job.get_dedicated_host_name());
		ClusterSimulation::file_logger->info("Job #{0} is dispatched to Host {1}", job.id, job.get_dedicated_host_name());
		Host& host = simulation_.find_host(job.get_dedicated_host_name());
		host.execute_job(job);
		simulation_.after_delay(job.run_time, [&host, job]
			{
				host.exit_job(job);
				std::stringstream ss(job.get_exit_host_status());
				ss >> Utils::enum_from_string<HostStatus>(host.status);
				spdlog::info("Job #{0} is finished in Host {1}. (run time: {2}ms)", job.id, host.name, 
					job.run_time.count());
				
			});
		*/
		
		//if (job.get_exit_host_status() == ""
	}

	//Doing filtering on candidate hosts
	//Match and sort(used priority queue)
	std::priority_queue<Host, std::vector<Host>, Queue::CompareHost> Queue::Match(const Job& job) const
	{	
		//지금은 cluster = candHostGroupList
		//To-Do : get_cluster --> Queue.get_host_group_list
		Cluster& cand_host_list{simulation_->get_cluster()};
		//std::vector<Host> eligible_host_list_;
		std::priority_queue<Host, std::vector<Host>, Queue::CompareHost> eligible_host_list_;
		
		for (Host& host : cand_host_list)
		{	
			//spdlog::info("host name {0}, host.max_slot {1} \n", host.name, host.max_slot);
			if(host.is_executable(job))
			{
				eligible_host_list_.push(host);
				//eligible_host_list_.push_back(host);
			}
		}

		//TO-DO : 찾지 못했을때
		if (eligible_host_list_.empty())
		{
			//PEND
			//(const Job -->  error (jobs_))
			//job.status = PEND;
			throw std::out_of_range("Can't find a host to dispatch");
			pending_jobs_.push_back(job);
		}
			
		return eligible_host_list_;
	}

	//job의 순서를 정함
	void Queue::policy()
	{
		//Scheduling policy
		//jobs_.push();
	}

	bool Queue::dispatch()
	{
		bool flag{ false };

	   // For all jobs in the current queue
		while (!jobs_.empty())
		{
			flag = true;
			// Pop a job from the inner queue;
			const Job& job{jobs_.top()};
			
			//spdlog::info("job is {}", job.id);
			//ClusterSimulation::file_logger->info("job is {}",job.id);
			std::priority_queue<Host, std::vector<Host>, Queue::CompareHost> eligible_hosts_ = Match(job);
			while (!eligible_hosts_.empty())
			{
				Host best_host = eligible_hosts_.top();

				//check if best_host is executable
				if(best_host.is_executable(job)){
					//spdlog::info("host is {}", host.name);
					ClusterSimulation::log(LogLevel::info, "Queue {0} dispatches Job #{1} to Host {2}"
						, name, job.id, best_host.name);
					
					//TO-DO : host.register()
					best_host.execute_job(job);
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

					jobs_.pop();
					break;

				}
				else{
					eligible_hosts_.pop();
					continue;
				}
			}
			//eligible_hosts_.empty()
			if(eligible_hosts_.empty()){
				throw std::out_of_range("Can't find a host to dispatch");
				pending_jobs_.push_back(job);
				jobs_.pop();
			}
			
		}

		return flag;
		
		
	}

	int Queue::id_gen_ = 0;

	//const std::map<std::string, int> queue_priorities;

}

