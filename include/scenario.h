#pragma once
#include <vector>
#include <chrono>
#include <queue>
#include <set>
#include <iostream>
#include <memory>

class Cluster;

namespace cs
{
	class ClusterSimulation;
	class Queue;
	enum class HostStatus;
	using ms = std::chrono::time_point<std::chrono::milliseconds>;

	struct ScenarioEntry
	{
		enum class Type
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

			double cpu_factor;
			int ncpus;
			int nprocs;
			int ncores;
			int nthreads;
		};


		Type type;
		ms timestamp;
		EventDetail event_detail;
		bool is_multi_host_submission;

		void print() const
		{
			std::cout <<
				//Utils::enum_to_string(type) <<
				timestamp.time_since_epoch().count() << std::endl;
		};

		std::shared_ptr<std::vector<unsigned short>> eligible_indices;
	};

	/**
	 * A container for 'Scenario Entry's.
	 * Represents a workload for clusters.
	 */
	class Scenario
	{
	public:
		ms initial_time_point;

		[[nodiscard]] std::size_t count() const { return entries_.size(); }
		[[nodiscard]] std::size_t num_unique_apps() const { return unique_apps_.size(); }
		[[nodiscard]] bool is_empty() const { return entries_.empty(); }

		const ScenarioEntry pop();
		[[nodiscard]] std::vector<ScenarioEntry> pop_all_latest();

		[[nodiscard]] std::vector<Queue> generate_queues(ClusterSimulation&) const;
		void add_scenario_entry(ScenarioEntry&& entry);

	private:
		std::queue<ScenarioEntry> entries_;
		std::set<std::string> unique_queues_;
		std::set<std::string> unique_apps_;
	};
}



