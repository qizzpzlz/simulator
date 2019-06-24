#pragma once
#include <vector>
#include <chrono>
#include <queue>
#include <set>
#include <iostream>
//#include "Cluster.h"

class Cluster;

namespace ClusterSimulator
{
	class ClusterSimulation;
	class Queue;
	enum class HostStatus;
	using ms = std::chrono::time_point<std::chrono::milliseconds>;

	struct ScenarioEntry
	{

		enum class ScenarioEntryType
		{
			CHANGE_STATUS,
			SUBMISSION
		};

		struct EventDetail
		{
			std::string host_name;
			HostStatus host_status;

			std::string queue_name;
			std::string exec_hostname;
			std::string application_name;
			long job_id;
			long mem_req;
			/// Always 1.
			int number_of_jobs;
			/// The number of processors that the job initially requested for execution.
			double num_exec_procs;
			/// The actual number of slots used for job execution.
			int num_slots;
			long job_mem_usage;
			long job_swap_usage;
			double job_cpu_time;
			double job_run_time;
			std::string job_exit_status;
			int job_exit_code;
			double job_non_cpu_time;
		};


		ScenarioEntryType type;
		ms timestamp;
		EventDetail event_detail;
		bool is_multi_host_submission;

		void print() const
		{
			std::cout <<
				//Utils::enum_to_string(type) <<
				timestamp.time_since_epoch().count() << std::endl;
		};


	};

	/**
	 * A container for 'Scenario Entry's.
	 * Represents a workload for clusters.
	 */
	class Scenario
	{
	public:
		//std::chrono::time_point<std::chrono::seconds> initial_time_point;
		ms initial_time_point;
		void add_scenario_entry(const ScenarioEntry& entry);
		int count() const { return entries_.size(); }
		int num_unique_apps() const { return unique_apps_.size(); }

		const ScenarioEntry pop();
		std::vector<Queue> generate_queues(ClusterSimulation&) const;
		bool is_empty() const { return entries_.empty(); }

	private:
		std::queue<ScenarioEntry> entries_;
		std::set<std::string> unique_queues_;
		std::set<std::string> unique_apps_;
	};
}



