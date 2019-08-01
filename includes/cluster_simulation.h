#pragma once
//#include "queue.h"
//#include "scenario.h"
#include "cluster.h"
#include <functional>
#include <chrono>
#include <queue>
#include <ctime>
#include "queue.h"
#include "../dependencies/spdlog/spdlog.h"
#include "../dependencies/spdlog/logger.h"
#include "../dependencies/spdlog/common.h"
#include "../dependencies/bprinter/include/bprinter/table_printer.h"

using LogLevel = spdlog::level::level_enum;
namespace ClusterSimulator
{
	struct ScenarioEntry;
	class Scenario;
	using ms = std::chrono::time_point<std::chrono::milliseconds>;

	class ClusterSimulation
	{
	public:
		using Action = std::function<void()>;
		struct EventItem
		{
			ms time;
			Action action;

			EventItem(ms time, Action action) 
				: time{ time }
				, action{ action } {};
			EventItem(const ScenarioEntry& entry, ClusterSimulation& simulation);
			/// Creates an event item for dispatching jobs
			explicit EventItem(ClusterSimulation& simulation);

			bool operator<(const EventItem& a) const { return a.time < this->time; }
		};
		std::chrono::milliseconds dispatch_frequency{ 1000 };

	private:
		Action dispatch_action_;
		ms current_time_;
		std::priority_queue<EventItem> events_{};

		void next();

	public:
		bool next_dispatch_reserved{ false };

		ClusterSimulation(Scenario& scenario, Cluster& cluster);

		ms get_current_time() const { return current_time_; }

		// TODO: use id instead of name
		Queue& find_queue(const std::string& name);
		Host& find_host(const std::string& name) const;
		const Cluster& get_cluster_view() const { return cluster_; }
		Cluster& get_cluster() const { return cluster_; }

		std::size_t event_count() const { return events_.size(); }
		void after_delay(std::chrono::milliseconds delay, Action block);
		bool run(std::chrono::time_point<std::chrono::milliseconds> run_time);
		bool run();
		
		void reserve_dispatch_event();

		void print_summary() const;

	private:
		Cluster& cluster_;
		Scenario& scenario_;
		std::vector<Queue> all_queues_{};
		
		// Stats
		int num_submitted_jobs_{ 0 };
		int num_successful_jobs_{ 0 };
		int num_failed_jobs_{ 0 };

#pragma region logger
	private:
		static std::shared_ptr<spdlog::logger> file_logger;
		static bprinter::TablePrinter tp_;
		static void initialise_tp();

	public:
		static std::ofstream jobmart_file_;

		template<typename... Args>
		static void log(LogLevel level, const char* fmt, const Args&... args)
		{
			spdlog::log(level, fmt, args...);
			file_logger->log(level, fmt, args...);
		}

		template<typename T>
		static void log(LogLevel level, const T& msg)
		{
			spdlog::log(level, msg);
			file_logger->log(level, msg);
		}

		static void log_jobmart(const Job& job)
		{

			tp_ <<
			// cluster_name
			// submit_time_gmt
			// start_time_gmt 
			// finish_time_gmt
			// submit_time
			// start_time
				job.start_time.time_since_epoch().count() <<
			// finish_time
				(job.start_time + job.run_time).time_since_epoch().count() <<
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
				job.get_dedicated_host_name() <<
			// exec_hosttype
			// exec_hostmodel
			// exec_hostgroup
			// num_exec_procs
				job.num_exec_procs <<
			// number_of_jobs
			// num_slots
				job.slot_required <<
			// job_exit_status
				job.get_exit_host_status() <<
			// job_exit_code
			// application_name
				job.get_application_name() <<
			// job_id
				job.id <<
			// job_array_index
			// job_name
			// job_cmd
			// job_pend_time
				job.total_pending_duration.count() <<
			// job_run_time
				job.run_time.count() <<
			// job_turnaround_time
			// job_mem_usage
				job.mem_usage <<
			// job_swap_usage
				job.swap_usage <<
			// job_cpu_time
				job.cpu_time <<
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
	};
#pragma endregion
}


