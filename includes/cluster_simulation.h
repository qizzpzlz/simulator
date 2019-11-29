#pragma once
//#include "queue.h"
//#include "scenario.h"
#include "cluster.h"
#include "scenario.h"
#include <functional>
#include <chrono>
#include <queue>
#include <ctime>
#include "queue.h"
#include "utils.h"
#include "../dependencies/spdlog/spdlog.h"
#include "../dependencies/spdlog/logger.h"
#include "../dependencies/spdlog/common.h"
#include "../dependencies/bprinter/include/bprinter/table_printer.h"
#include <map>

using LogLevel = spdlog::level::level_enum;
namespace ClusterSimulator
{
	class Scenario;
	struct ScenarioEntry;

	class ClusterSimulation
	{
		inline static const std::string PERFORMANCE_FILE_NAME = "performance.txt";
		inline static const std::string PENDING_FILE_NAME = "pending.txt";
		inline static const std::string JOB_SUBMIT_FILE_NAME = "job_submit.txt";
		static constexpr bool console_output = true;
		static constexpr bool console_warning_output = false;
		static constexpr bool log_file_output = false;
		static constexpr bool jobmart_file_output = false;
		static constexpr bool slots_file_output = true;

	public:
		using Action = std::function<void()>;
		struct EventItem
		{
			ms time;
			Action action;
			int priority;

			EventItem(ms time, Action action, int priority = 0) 
				: time{ time }
				, action{ action }
				, priority{ priority }{}

			EventItem(const ScenarioEntry& entry, ClusterSimulation& simulation);

			bool operator<(const EventItem& a) const
			{
				return a.time == time 
					? a.priority < priority 
					: a.time < time;
			}
		};
		std::chrono::milliseconds dispatch_frequency{ 1000 };
		std::chrono::milliseconds logging_frequency{ 10000 };
		std::chrono::milliseconds counting_frequency{ 10000 };
	
	private:
		ms current_time_;
		std::priority_queue<EventItem> events_{};
		Action log_action_;
		Action count_new_jobs_;

		std::unordered_map<ms, std::size_t, Utils::ms_hash> job_submit_record_;
		std::unordered_map < ms, std::size_t , Utils::ms_hash > using_slot_record_;
		std::vector<std::pair<ms, std::size_t>> pending_record_;
		ms latest_finish_time_;

		void next();

	public:
		ClusterSimulation(Scenario& scenario, Cluster& cluster, const QueueAlgorithm& algorithm);

		bool next_dispatch_reserved{ false };
		std::size_t num_dispatched_slots{ 0 };

		ms get_current_time() const { return current_time_; }

		// TODO: use id instead of name
		Queue& find_queue(const std::string& name);
		const Host& find_host(const std::string& name) const;
		const Cluster& get_cluster_view() const { return cluster_; }
		Cluster& get_cluster() const { return cluster_; }

		std::size_t event_count() const { return events_.size(); }
		void after_delay(std::chrono::milliseconds delay, Action block, int priority = 0);
		bool run(std::chrono::time_point<std::chrono::milliseconds> run_time);
		bool run();
		
		void reserve_dispatch_event();

		void print_summary() const;

		constexpr void update_latest_finish_time(ms time) noexcept { latest_finish_time_ = time; }
		constexpr void increment_failed_jobs() noexcept { ++num_failed_jobs_; }
		constexpr void update_pending_duration(std::chrono::milliseconds duration) { total_pending_duration_ += duration; }

	private:
		Cluster& cluster_;
		Scenario& scenario_;
		std::vector<Queue> all_queues_{};
		
		// Stats
		std::size_t num_submitted_jobs_{ 0 };
		std::size_t newly_submitted_jobs_{ 0 };
		std::size_t num_successful_jobs_{ 0 };
		std::size_t num_failed_jobs_{ 0 };
		std::size_t num_pending_jobs_{ 0 };
		std::chrono::milliseconds total_pending_duration_{};

		class Dispatcher
		{
			ClusterSimulation* simulation_;
			std::size_t latest_cluster_version_{ };

		public:
			explicit Dispatcher(ClusterSimulation* simulation) : simulation_{simulation}{} 
			void operator()()
			{
				auto version{ simulation_->cluster_.get_version() };
				if (version == latest_cluster_version_)
				{
					if (simulation_->scenario_.count() == 0 && simulation_->num_pending_jobs_ == 0)
					{
						simulation_->next_dispatch_reserved = false;
						return;
					}

					simulation_->after_delay(simulation_->dispatch_frequency,
						std::ref(simulation_->dispatcher_), 1);
					return;
				}
				latest_cluster_version_ = version;

				bool flag{ false };
				for (auto& q : simulation_->all_queues_)
					flag |= q.dispatch();
				if (flag)
				{
					// pending jobs exist
					simulation_->after_delay(simulation_->dispatch_frequency,
						std::ref(simulation_->dispatcher_), 1);

					size_t num{ 0 };
					for (const auto& q : simulation_->all_queues_)
						num += q.get_num_pending_jobs();
					simulation_->num_pending_jobs_ = num;
				}
				else
				{
					simulation_->next_dispatch_reserved = false;
					latest_cluster_version_ = 0;
				}


				simulation_->log_using_slots();
			}	
		};

		Dispatcher dispatcher_;

#pragma region logger
	private:
		std::ofstream jobmart_file_ = []{
			if constexpr (jobmart_file_output){
				return std::ofstream()
			}
		}
		
		std::shared_ptr<spdlog::logger> file_logger;
		bprinter::TablePrinter tp_;
		void initialise_tp();

	public:


		template<typename... Args>
		void log(LogLevel level, const char* fmt, const Args&... args)
		{
			if constexpr (console_output)
				spdlog::log(level, fmt, args...);
			else if constexpr (console_warning_output)
				if (level > LogLevel::info)
					spdlog::log(level, fmt, args...);
			if constexpr (log_file_output)
				file_logger->log(level, fmt, args...);
		}

		template<typename T>
		static void log(LogLevel level, const T& msg)
		{
			if constexpr (console_output)
				spdlog::log(level, msg);
			else if (level > LogLevel::info)
				spdlog::log(level, msg);
			if constexpr (log_file_output)
				file_logger->log(level, msg);
		}

		void log_jobmart(const Job& job)
		{
			if constexpr (!jobmart_file_output)
				return;

			tp_ <<
			// cluster_name
			// submit_time_gmt
			// start_time_gmt 
			// finish_time_gmt
			// submit_time
			// start_time
				job.start_time.time_since_epoch().count() <<
			// finish_time
				job.finish_time.time_since_epoch().count() <<
			// FINISH_ISO_WEEK
			// project_name
			// queue_name
				job.queue_managing_this_job->name <<
			// user_group
			// user_name
			// job_type
			// job_group
			// sla_tag
			// res_req
			// submission_host
			// exec_hostname
				// job.get_dedicated_host_name() <<
				job.get_run_host_name() <<
			// exec_hosttype
			// exec_hostmodel
			// exec_hostgroup
			// num_exec_procs
				// job.num_exec_procs <<
			// number_of_jobs
			// num_slots
				job.slot_required <<
			// job_exit_status
				// job.get_exit_host_status() <<
			// job_exit_code
			// application_name
				// job.get_application_name() <<
			// job_id
				job.id <<
			// job_array_index
			// job_name
			// job_cmd
			// job_pend_time
				job.total_pending_duration.count() <<
			// job_run_time
				(job.finish_time - job.start_time).count() <<
			// job_turnaround_time
			// job_mem_usage
				// job.mem_usage <<
			// job_swap_usage
				// job.swap_usage <<
			// job_cpu_time
				// job.cpu_time <<
			// pend_time
			// run_time
			// turnaround_time
			// mem_usage
			// swap_usage
			// cpu_time
			// rank_mem
			// rank_mem_req
			// rank_runtime
			// rank_pendtime
			// rank_cputime
			// rank_efficiency
			// job_group1
			// job_group2
			// job_group3
			// job_group4
			// cluster_mapping
			// job_description
			// exit_reason
			// run_limit
			// begin_time
			// depend_cond
				
			bprinter::endl();
		}

		void log_using_slots() 
		{ 
			if constexpr (!slots_file_output) return;

			using_slot_record_.insert_or_assign(get_current_time(), num_dispatched_slots);
			pending_record_.emplace_back(get_current_time(), num_pending_jobs_);
		}
	};
#pragma endregion
}


