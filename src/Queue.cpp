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
	std::map<std::string, int> Queue::StaticQueueData::queue_priorities;

	Queue::Queue(ClusterSimulation& simulation) :  name{ "normal" }, priority{ DEFAULT_PRIORITY }, simulation_{ simulation }
	{
		is_default_ = true;
	}

	Queue::Queue(ClusterSimulation& simulation, int priority, std::string name) 
	:  name{ std::move(name) }, priority{ priority }, simulation_{ simulation }
	{

	}

	Queue::Queue(ClusterSimulation& simulation, const std::string& name) 
	: name{name}, priority{StaticQueueData::queue_priorities[name]}, simulation_{ simulation }
	{
		
	}


	Queue::~Queue()
	= default;

	void Queue::enqueue(Job&& job)
	{
		const std::string& job_name = job.get_dedicated_host_name();

		if (job_name == "-" || job_name.empty())
			return;

		jobs_.push_back(job);
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

	Host& Queue::policy(Job& job)
	{
		
		std::vector<Host> host_list = simulation_.all_host();
		int score; 
		Host& best_host= simulation_.find_host(job.get_dedicated_host_name()); 
		int best_score = best_host.max_slot - best_host.num_current_running_slots + best_host.max_mem + best_host.nprocs + best_host.max_swp;
		spdlog::info("Ori best_Score {0}, host{1}", best_score, best_host.name);
		ClusterSimulation::file_logger->info("Ori best_Score {0}, host{1}", best_score, best_host.name);
			
		//mutihost jobs
		//if(job.is_multi_host)(
		//  
		//	return host[];
		//)


		//직접 호스트를 지정해준 경우
		//if (!job.get_dedicated_host_name().empty()){
		//	return simulation_.find_host(job.get_dedicated_host_name());
		//}


		for (auto& host : host_list)
		{	
			//spdlog::info("host name {0}, host.max_slot {1} \n", host.name, host.max_slot);
			if(host.status==HostStatus::OK 
			&& host.num_current_running_slots + job.slot_required < host.max_slot 
			&& job.mem_required < host.max_mem
			&& job.num_exec_procs < host.nprocs
			&& job.swap_usage < host.max_swp
			
			)
			{
				//spdlog::info("avail hosts{}", host.name);
				//ClusterSimulation::file_logger->info("avail hosts{}", host.name);

				score = host.max_slot - host.num_current_running_slots + host.max_mem + host.nprocs + host.max_swp;
				//TO-DO : score = cpu factor here ..? 
				if(score> best_score){
					best_score = score;
					//best_host= host;
					spdlog::info("best_Score updated{0}, host{1}", best_score, host.name);
					ClusterSimulation::file_logger->info("hbest_Score updated{0}, host{1}", best_score, host.name);
				}
				
			}
		}	

		//return *best_host;
		//TO-DO : 찾지 못했을때
		//throw std::out_of_range("Can't find a host to dispatch");
		return best_host;
		
	}	
	void Queue::dispatch()
   {
	   // For all jobs in the current queue
		while (!jobs_.empty())
		{
			// Pop a job from the inner queue;
			Job& job = jobs_.back();
			jobs_.pop_back();
			
			spdlog::info("job is {}", job.id);
			ClusterSimulation::file_logger->info("job is {}",job.id);
			
			Host& host = policy(job);
			//spdlog::info("host is {}", host.name);
			
			//TO-DO : host.register()
			host.execute_job(job);
			simulation_.after_delay(job.run_time, [&host, job]
			{
				host.exit_job(job);
				std::stringstream ss(job.get_exit_host_status());
				ss >> Utils::enum_from_string<HostStatus>(host.status);
				spdlog::info("Job #{0} is finished in Host {1}. (run time: {2}ms)", job.id, host.name, 
					job.run_time.count());
				
			});

		}
		
		/* 
		// Find the best host
		Host host = policy(job);
		spdlog::info("dispatch");
		ClusterSimulation::file_logger->info("dispatch");
		*/
		
		

   }

	int Queue::id_gen_ = 0;

	//const std::map<std::string, int> queue_priorities;

}

