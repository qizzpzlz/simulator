#include <utility>
#include <sstream>
#include "../includes/Host.h"
#include "../includes/Queue.h"
#include "../includes/Job.h"
#include "../includes/Cluster_Simulation.h"
#include "../dependencies/spdlog/spdlog.h"

//#include "spdlog/spdlog.h"

namespace ClusterSimulator
{
	std::map<std::string, int> Queue::StaticQueueData::queue_priorities;

	Queue::Queue(ClusterSimulation& simulation) : name{ "normal" }, priority{ DEFAULT_PRIORITY }, simulation_{ simulation }
	{
		is_default_ = true;
	}

	Queue::Queue(ClusterSimulation& simulation, int priority, std::string name) : name{ std::move(name) }, priority{ priority }, simulation_{ simulation }
	{

	}

	Queue::Queue(ClusterSimulation& simulation, const std::string& name) : name{name}, priority{StaticQueueData::queue_priorities[name]}, simulation_{ simulation }
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

		// TODO: instant dispatch
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
		
		//if (job.get_exit_host_status() == ""
	}

	int Queue::id_gen_ = 0;

	//const std::map<std::string, int> queue_priorities;

}

