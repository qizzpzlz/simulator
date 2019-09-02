#pragma once
#include <string>
#include <chrono>
#include <memory>

namespace ClusterSimulator
{
	struct ScenarioEntry;
	enum class JobState
	{
		PEND, RUN, DONE, EXIT, PSUSP, USUSP, SSUSP, POST_DONE, POST_ERR, UNKWN, WAIT, ZOMBI
	};
	class Queue;
	//enum class HostStatus;

	using ms = std::chrono::time_point<std::chrono::milliseconds>;

	class Job
	{
	public:
		int id{ id_gen_++ };
		// The actual number of slots used for job execution.
		int slot_required;
		std::chrono::milliseconds run_time;
		//std::shared_ptr<const Queue> queue_managing_this_job;
		Queue* queue_managing_this_job;
		bool is_multi_host;
		long mem_required;
		long swap_usage;
		// The number of processors that the job initially requested for execution.
		double num_exec_procs;

		ms submit_time;
		ms start_time{};
		ms finish_time{};

		std::chrono::milliseconds total_pending_duration{};
		void update_total_pending_duration(ms current_time) { total_pending_duration = current_time - pend_start_time_; }

		int priority{ 0 };
		JobState state{ JobState::WAIT };

		Job(const ScenarioEntry& entry, Queue& queue, const ms submit_time);

		const std::string& get_application_name() const { return application_name_; }
		const std::string& get_dedicated_host_name() const { return dedicated_host_name_; }
		const std::string& get_exit_host_status() const { return exit_host_status_; }
		void set_pending(ms time) noexcept
		{
			state = JobState::PEND;
			pend_start_time_ = time;
		}

		long mem_usage;
		double cpu_time;

	private:
		std::string application_name_;
		std::string dedicated_host_name_;
		std::string exit_host_status_;

		ms pend_start_time_{};

		//unsigned long id_;
		//double submitted_time_;
		//int num_cpu_required_;
		//int task_index_;
		//std::vector<Task> tasks_;

		static int id_gen_;
	};
}


