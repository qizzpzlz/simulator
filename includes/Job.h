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

	using ms = std::chrono::time_point<std::chrono::milliseconds>;

	class Job
	{
	public:
		int id{ id_gen_++ };
		//std::string application_name;
		int slot_required;
		std::chrono::milliseconds run_time;
		std::shared_ptr<const Queue> queue_managing_this_job;
		bool is_multi_host;
		long mem_required;
		long swap_usage;
		double num_exec_procs;

		ms submit_time;
		mutable ms start_time{};
		mutable ms finish_time{};

		Job(const ScenarioEntry& entry, const Queue& queue, const ms submit_time);

		const std::string& get_application_name() const { return application_name_; }
		const std::string& get_dedicated_host_name() const { return dedicated_host_name_; }
		const std::string& get_exit_host_status() const { return exit_host_status_; }

		long mem_usage;
		double cpu_time;

	private:
		std::string application_name_;
		std::string dedicated_host_name_;
		std::string exit_host_status_;

		//unsigned long id_;
		//double submitted_time_;
		//int num_cpu_required_;
		//int task_index_;
		//std::vector<Task> tasks_;

		static int id_gen_;
	};
}


