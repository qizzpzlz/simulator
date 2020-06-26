#pragma once
#include "config.h"
#include "event_item.h"
#include "cluster.h"
#include "scenario.h"
#include "queue.h"
#include "utils.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "bprinter/include/bprinter/table_printer.h"
#include <functional>
#include <chrono>
#include <queue>
#include <ctime>
#include <filesystem>
#include <fstream>


using LogLevel = spdlog::level::level_enum;

namespace cs
{
	class Scenario;
	struct ScenarioEntry;
	using std::chrono::milliseconds;

	class ClusterSimulation
	{
		friend class Scenario;
		friend class Host;

	public:
		static constexpr bool LOG_ANY = config::CONSOLE_OUTPUT || config::LOG_FILE_OUTPUT;


	private:
		/* Event driven simulator helpers */
		
		ms current_time_;
		Utils::EventQueue<EventItem> events_{};
		Action log_action_;
		Action count_new_jobs_;

		/* Records */
		
		std::unordered_map<ms, std::size_t, Utils::ms_hash> job_submit_record_;
		std::unordered_map<ms, std::size_t , Utils::ms_hash> using_slot_record_;
		std::vector<std::pair<ms, std::size_t>> pending_record_;
		ms latest_finish_time_;
		duration<double> actual_run_time_{};
		
		void next();

	public:
		ClusterSimulation(Scenario& scenario, Cluster& cluster, const QueueAlgorithm& algorithm);

		bool next_dispatch_reserved{ false };
		std::size_t num_dispatched_slots{ 0 };

		[[nodiscard]] ms get_current_time() const { return current_time_; }

		[[nodiscard]] Queue& get_default_queue() { return all_queues_[0]; }
		// TODO: use id instead of name
		[[nodiscard]] Queue& find_queue (const std::string& name);
		[[nodiscard]] const Host& find_host(const std::string& name) const;
		[[nodiscard]] const Cluster& get_cluster_view() const { return cluster_; }
		[[nodiscard]] Cluster& get_cluster() const { return cluster_; }
		
		[[nodiscard]] std::size_t event_count() const { return events_.size(); }

		/**
		  * Reserve an event after a specified duration.
		  * Return: id of the event.
		  */
		std::size_t after_delay(milliseconds delay, Action block, uint8_t priority = 0, EventItem::Type type = EventItem::Type::SCENARIO)
		{
			EventItem event_item{ current_time_ + delay, std::move(block), priority, type };
			events_.push(event_item);

			if constexpr (config::DEBUG_EVENTS)
			{
				log(LogLevel::debug, "Event [{0}] is added at {1} ms. Event time: {2} ms",
					event_item.get_type_string(), current_time_.time_since_epoch().count(),
					event_item.time.time_since_epoch().count());
			}

			return event_item.id;
		}
		
		bool run();

		void erase_event(std::size_t event_id)
		{
			auto it = events_.find_by_id(event_id);
			if (it == events_.end()) return;

			if constexpr (config::DEBUG_EVENTS)
			{
				log(LogLevel::debug, "Event [{0}] is removed. (was planed to start at {1} ms.)"
					, it->get_type_string(), it->time.time_since_epoch().count());
			}
			
			events_.erase(it);
		}

		void add_delay(std::size_t event_id, milliseconds delay)
		{
			events_.add_delay(event_id, delay);
			if constexpr (config::DEBUG_EVENTS)
			{
				const auto event = events_.find_by_id(event_id);
				log(LogLevel::debug, "Event [{0}]'s start time is changed to {1} ms.",
					event->get_type_string(), event->time.time_since_epoch().count());
			}
		}
		
		void reserve_dispatch_event();

		void print_summary();

		constexpr void update_latest_finish_time(ms time) noexcept { latest_finish_time_ = time; }
		constexpr void increment_failed_jobs() noexcept { ++num_failed_jobs_; }
		constexpr void update_pending_duration(milliseconds duration) { total_pending_duration_ += duration; }
		constexpr void update_total_queuing_time(milliseconds q_time) { total_queuing_time_ += q_time; }

		void increment_job_submission_counters() { ++num_submitted_jobs_; ++newly_submitted_jobs_; }

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
		milliseconds total_pending_duration_{};
		milliseconds total_queuing_time_{};

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

					simulation_->after_delay(config::DISPATCH_FREQUENCY,
						std::ref(simulation_->dispatcher_), 1, EventItem::Type::DISPATCH);
					return;
				}
				latest_cluster_version_ = version;

				bool flag{ false };
				for (auto& q : simulation_->all_queues_)
					flag |= q.dispatch();
				if (flag)
				{
					// pending jobs exist
					simulation_->after_delay(config::DISPATCH_FREQUENCY,
						std::ref(simulation_->dispatcher_), 1, EventItem::Type::DISPATCH);

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

#pragma pack(push, 1)
		struct _Allocation_record_entry
		{
			uint64_t job_id;
			uint16_t host_id;
			int64_t start_time;

			_Allocation_record_entry(const Job& job, int host_id) noexcept
				: job_id{ job.id }
				, host_id{ static_cast<uint16_t>(host_id) }
				, start_time{ job.start_time.time_since_epoch().count() } {}
		};
#pragma pack(pop)
		std::vector<_Allocation_record_entry> allocation_records_;

		/**
		 * Log a job allocation record for the allocation binary output.
		 */
		void log_allocation(const Job& job, int host_id)
		{
			allocation_records_.emplace_back(job, host_id);
		}
		/**
		 * Generate an allocation binary file.
		 * The binary file contains the following information:
		 * - Allocated host for each processed job.
		 * - The time of execution for each processed job.
		 */
		void generate_allocation_binary(std::string_view output_path = "allocation.bin")
		{
			using record_t = const _Allocation_record_entry;

			// Sort records by job id
			std::sort(allocation_records_.begin(), allocation_records_.end(),
				[](record_t& a, record_t& b) { return a.job_id < b.job_id; });

			std::filebuf output_stream;
			output_stream.open(output_path, std::ios::out | std::ios::binary);

			for (auto& record : allocation_records_)
				output_stream.sputn(reinterpret_cast<const char*>(&record), sizeof(record_t));
		}

#pragma region logger
	private:
		inline static bool _set_log_level = []
		{
			if constexpr (!config::DEBUG_EVENTS) return false;
			spdlog::set_level(LogLevel::debug);
			return true;
		}();
		
		static std::ofstream generate_file(std::string_view directory, std::string_view file)
		{
			return std::ofstream(std::filesystem::path(directory) / file);
		}

		std::ofstream jobmart_file_ = [] {
			if constexpr (config::JOBMART_FILE_OUTPUT)
				return generate_file(config::LOG_DIRECTORY, config::JOBMART_FILE_NAME);
			else
				return std::ofstream{};
		}();
		std::ofstream performance_file_ = []
		{
			if constexpr (config::SLOTS_FILE_OUTPUT)
				return generate_file(config::LOG_DIRECTORY, config::PERFORMANCE_FILE_NAME);
			else
				return std::ofstream{};
		}();
		std::ofstream pending_jobs_file_ = []
		{
			if constexpr (config::SLOTS_FILE_OUTPUT)
				return generate_file(config::LOG_DIRECTORY, config::PENDING_FILE_NAME);
			else
				return std::ofstream{};
		}();
		std::ofstream job_submit_file_ = []
		{
			if constexpr (config::JOB_SUBMIT_FILE_OUTPUT)
				return generate_file(config::LOG_DIRECTORY, config::JOB_SUBMIT_FILE_NAME);
			else
				return std::ofstream{};
		}();
		std::shared_ptr<spdlog::logger> file_logger_ = []
		{
			if constexpr (config::LOG_FILE_OUTPUT)
			{
				std::filesystem::path path = config::LOG_DIRECTORY;
				path /= config::LOG_OUTPUT_FILE_NAME;
				auto logger = spdlog::basic_logger_mt("file_logger", path.string());
				logger->set_pattern(config::LOGGER_PATTERN);
				return logger;
			}
			else
				return std::shared_ptr<spdlog::logger>();
		}();

		bprinter::TablePrinter tp_{ &jobmart_file_ };
		void initialise_tp();

	public:
		template<typename... Args>
		void log(LogLevel level, const char* fmt, const Args&... args) const
		{
			if constexpr (config::CONSOLE_OUTPUT)
				spdlog::log(level, fmt, args...);
			else if constexpr (config::CONSOLE_WARNING_OUTPUT)
				if (level > LogLevel::info)
					spdlog::log(level, fmt, args...);
			if constexpr (config::LOG_FILE_OUTPUT)
				file_logger_->log(level, fmt, args...);
		}

		template<typename T>
		void log(LogLevel level, const T& msg) const
		{
			 if constexpr (config::CONSOLE_OUTPUT)
			 	spdlog::log(level, msg);
			 else if constexpr (config::CONSOLE_WARNING_OUTPUT) 
			 	if (level > LogLevel::info)
			 		spdlog::log(level, msg);
			 if constexpr (config::LOG_FILE_OUTPUT)
			 	file_logger_->log(level, msg);
		}

		void log_jobmart(const Job& job)
		{
			if constexpr (!config::JOBMART_FILE_OUTPUT)
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
			if constexpr (!config::SLOTS_FILE_OUTPUT) return;

			using_slot_record_.insert_or_assign(get_current_time(), num_dispatched_slots);
			pending_record_.emplace_back(get_current_time(), num_pending_jobs_);
		}
	};
#pragma endregion
}


