#pragma once
#include <string>
#include <chrono>
#include <memory>
//#include "Scenario.h"

namespace ClusterSimulator
{
	struct ScenarioEntry;
	/*enum JobState
	{
		PEND, RUN, DONE, EXIT, PSUSP, USUSP, SSUSP, POST_DONE, POST_ERR, UNKWN, WAIT, ZOMBI
	};*/
	class Queue;
	//enum class HostStatus;

	class Job
	{
	public:
		const int id{ id_gen_++ };
		const std::string application_name;
		const int slot_required;
		const std::chrono::milliseconds run_time;
		//std::shared_ptr<Queue> queue_managing_this_job;
		bool is_multi_host;
		const long mem_required;
		const long swap_usage;
		const double num_exec_procs;

		Job(const ScenarioEntry& entry, const Queue& queue);

		const std::string& get_dedicated_host_name() const { return dedicated_host_name_; }
		const std::string& get_exit_host_status() const { return exit_host_status_; }

	private:
		const std::string dedicated_host_name_;
		const std::string exit_host_status_;
		//unsigned long id_;
		//double submitted_time_;
		//int num_cpu_required_;
		//int task_index_;
		//std::vector<Task> tasks_;

		static int id_gen_;
	};
}


