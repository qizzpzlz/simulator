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
	void Queue::enqueue(Job&& job)
	{
		//const std::string& job_name{ job.get_dedicated_host_name() };

		//// TODO
		//if (job_name == "-" || job_name.empty())
		//	return;

		//jobs_.push_back(job);

		//ClusterSimulation::log(LogLevel::info, 
		//	"Job #{0} is added to Queue {1}.", job.id, this->name);

		jobs_.push_back(std::make_shared<Job>(job));

		ClusterSimulation::log(LogLevel::info,
			"Job #{0} is added to Queue {1}.", job.id, this->name);
	}

	/**
	  * Returns a list of hosts being able to execute the given job.
	  */
	std::vector<Host*> Queue::match(const Job& job)
	{	
		//// Currently cluster == candHostGroupList
		//// To-Do : get_cluster --> Queue.get_host_group_list
		//Cluster& cand_host_list{ simulation_->get_cluster() };

		//// TODO: Reuse host vector instead of allocating memory each time.
		//HostList eligible_host_list{};

		//for (auto & [name, host] : cand_host_list)
		//{	
		//	if(host.is_executable(job))
		//	{
		//		bool flag{ false };
		//		for (auto& limit : limits)
		//		{
		//			if (!limit->is_eligible(*this, host, job))
		//			{
		//				flag = true;
		//				break;
		//			}
		//		}
		//		if (flag) continue;

		//		eligible_host_list.push_back(&host);
		//	}
		//}

		//return eligible_host_list;

		//simulation_->get_cluster().
		//std::vector<Host>& cand_host_list{ simulation_->get_cluster().vector() };
		Cluster& cluster{ simulation_->get_cluster() };

		// TODO: Reuse host vector instead of allocating memory each time.
		std::vector<Host*> eligible_host_list{};

		int max = omp_get_max_threads();
		std::vector <std::vector<Host* >> hosts(max);

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

				//eligible_host_list.push_back(&host);
				hosts[tid].push_back(&host);
			}
		}

		for (auto& vec : hosts)
			eligible_host_list.insert(eligible_host_list.end(), vec.begin(), vec.end());

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
		//// Returns true if any jobs are dispatched this time.
		//bool flag{ false };

		//// 1. Bring back all pending jobs.
		//clean_pending_jobs();

		//// 2. Sort all jobs using policy.
		//policy();

		//// For each job in the current queue
		//// find the best host and dispatch to it or
		//// delay it.
		//while (!jobs_.empty())
		//{
		//	// Peek a job from the inner queue;
		//	Job& job{ jobs_.back() };

		//	// TODO: Multi-host jobs
		//	if (job.is_multi_host)
		//	{
		//		jobs_.pop_back();
		//		continue;
		//	}

		//	// Check qjob_limit here.
		//	if(qjob_limit != -1 && using_job_slots > qjob_limit )
		//	{
		//		ClusterSimulation::log(LogLevel::info, "Reached qjob_limit");
		//		job.set_pending(simulation_->get_current_time());
		//		pending_jobs_.push_back(job);
		//		ClusterSimulation::log(LogLevel::info, "Job #{0} is pended. (pending duration: {1}ms)"
		//						, job.id, job.total_pending_duration.count());
		//		

		//		jobs_.pop_back();
		//		continue;
		//	}
		//	Host* best_host;

		//	if (current_algorithm != nullptr)
		//	{
		//		// Get all avilable hosts for the current job.
		//		Queue::HostList eligible_hosts{ match(job) };
		//		
		//		// If there is no eligible host for this job,
		//		// pend the job.
		//		if (eligible_hosts.empty())
		//		{
		//			ClusterSimulation::log(LogLevel::info, "Job #{0} is pended. (pending duration: {1}ms)"
		//							, job.id, job.total_pending_duration.count());
		//			job.set_pending(simulation_->get_current_time());
		//			pending_jobs_.push_back(job);

		//			jobs_.pop_back();
		//			continue;
		//		}

		//		// TODO: Why would we sort hosts? 
		//		// Can we just get the host with the maximum priority?
		//		// sort(eligible_hosts.begin(), eligible_hosts.end(), job);

		//		// best_host = eligible_hosts.front();
		//		using namespace std::placeholders;
		//		auto f = std::bind(compare_host_function_, _1, _2, job);
		//		auto it = std::min_element(eligible_hosts.begin(), eligible_hosts.end(), f);
		//		best_host = *it;
		//	}
		//	else
		//	{
		//		best_host = &simulation_->get_cluster().find_node(job.get_dedicated_host_name())->second;
		//	}
		//	

		//	// Find best available host
		//	// Host& best_host{ *eligible_hosts.back() };
		//	
		//	//limits
		//	//if(cpu_limit != -1 && total_cpu_time > cpu_limit)
		//	//{
		//		//CPU_LIMIT ..?? 
		//	
		//	// Register the best host to the dispatched hosts list.
		//	// auto search = dispatched_hosts_.find(best_host);
		//	// if (search != dispatched_hosts_.end())
		//	// {
		//	// 	dispatched_hosts_[best_host].slot_dispatched += job.slot_required;
		//	// 	// ClusterSimulation::log(LogLevel::info, "else ++job {0}: {1} = slot_dis {2}, queue name :{3}"
		//	// 	// ,job.id, job.slot_required, dispatched_hosts_[&best_host].slot_dispatched, name);
		//	// 	// ClusterSimulation::log(LogLevel::info, "++ cnt : {0}, queue name : {1}" , using_job_slots(), name);	
		//
		//	// }
		//	// else
		//	// {
		//	// 	dispatched_hosts_.insert(std::make_pair(best_host, HostInfo{job.slot_required}));
		//	// 	// ClusterSimulation::log(LogLevel::info, "if ++job {0}: {1} = slot_dis {2}, queue name :{3}"
		//	// 	// ,job.id, job.slot_required, dispatched_hosts_[&best_host].slot_dispatched, name);
		//	// }

		//	simulation_->num_dispatched_slots += job.slot_required;
		//	using_job_slots += job.slot_required;
		//	flag = true;
		//	
		//	ClusterSimulation::log(LogLevel::info, "Queue {0} dispatches Job #{1} to Host {2}"
		//		,name, job.id, best_host->get_name());

		//	const auto run_time = best_host->get_expected_run_time(job);
		//	best_host->try_update_expected_time_of_completion(run_time);

		//	// TODO: host.register()
		//	best_host->execute_job(job);
		//	
		//	// TODO: Host.set_start_time()
		//	const ms start_time = simulation_->get_current_time();
		//	job.start_time = start_time;
		//	job.finish_time = start_time + run_time;

		//	// Reserve finish event
		//	simulation_->after_delay(run_time,  [best_host, job, this] () mutable -> void
		//	{
		//		//ClusterSimulation::log(LogLevel::info, "oo cnt : {0}, queue name: {1}" , using_job_slots(), name);
		//		best_host->exit_job(job);
		//		job.set_run_host_name(best_host->get_name());
		//		std::stringstream ss(job.get_exit_host_status());
		//		ss >> Utils::enum_from_string<HostStatus>(best_host->status);
		//		ClusterSimulation::log(LogLevel::info, 
		//			"Job #{0} is finished in Host {1}. (actual run time: {2} ms, scenario run time: {3} ms)"
		//			,job.id, best_host->get_name(), (job.finish_time - job.start_time).count(), job.run_time.count());
		//		
		//		// dispatched_hosts_[best_host].slot_dispatched -= job.slot_required;
		//		this->simulation_->num_dispatched_slots -= job.slot_required;
		//		this->using_job_slots -= job.slot_required;
		//	
		//		// ClusterSimulation::log(LogLevel::info, "--job {0}: {1} = slot_dis {2}, queue name :{3}"
		//		// ,job.id, job.slot_required, dispatched_hosts_[&best_host].slot_dispatched, name);
		//		//ClusterSimulation::log(LogLevel::info, "-- cnt : {0}, queue name :{1}" , using_job_slots(), name);

		//		this->simulation_->log_using_slots();
		//		ClusterSimulation::log_jobmart(job);
		//	});

		//	
		//	jobs_.pop_back();
		//}
		//// Returns true if there exists any pending jobs
		//return flag;


		// Bring back all pending jobs.
		clean_pending_jobs();

		current_algorithm->run(jobs_);
		while (!jobs_.empty())
		{
			std::shared_ptr<Job>& job{ jobs_.back() };

			// Set pending for not assigned jobs
			if (job->state != JobState::RUN)
			{
				job->set_pending(simulation_->get_current_time());
				pending_jobs_.push_back(job);
				ClusterSimulation::log(LogLevel::info, "Job #{0} is pended. (pending duration: {1}ms)"
					, job->id, job->total_pending_duration.count());
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
					, job->id, job->slot_required);
			}
		}

		pending_jobs_.clear();
	}

	int Queue::id_gen_ = 0;
}

