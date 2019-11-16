#include <iostream>
#include <sstream>
#include "../includes/queue.h"
#include "../includes/cluster.h"
#include "../includes/scenario.h"
#include "../includes/cluster_simulation.h"
#include "../includes/job.h"
#include "../dependencies/spdlog/spdlog.h"

//#include "spdlog/spdlog.h"
#include "../dependencies/spdlog/sinks/basic_file_sink.h"
#include "../dependencies/spdlog/common.h"
#include <fstream>

//#include "dependencies/

namespace ClusterSimulator
{
	std::shared_ptr<spdlog::logger> ClusterSimulation::file_logger;
	std::ofstream ClusterSimulation::jobmart_file_;
	std::ofstream performance_("logs/performance_.txt");
	std::ofstream pending_jobs_file("logs/pending.txt");
	std::ofstream job_submit_("logs/job_submit_.txt");
	bprinter::TablePrinter ClusterSimulation::tp_{ &jobmart_file_ };

	ClusterSimulation::ClusterSimulation(Scenario& scenario, Cluster& cluster, const QueueAlgorithm& algorithm)
	: cluster_{ cluster },
	scenario_{ scenario },
	all_queues_{ scenario.generate_queues(*this) },
	dispatcher_{ this }
	{
		cluster.simulation = this;

		// TODO: set default queue

		current_time_ = scenario.initial_time_point;
		// std::sort(all_queues_.begin(), all_queues_.end());
		all_queues_[0].set_algorithm(&algorithm);
		
		for (auto& host : cluster.vector())
		{
			host.simulation = this;
		}
		
		count_new_jobs_ = [this]
		{
			job_submit_record_.insert_or_assign(this->get_current_time(), newly_submitted_jobs_);

			
			if (newly_submitted_jobs_ > 0)
			{
				log(LogLevel::info, "total_new_jobs: {0}", newly_submitted_jobs_);
				newly_submitted_jobs_ = 0;
			}

			if (!scenario_.is_empty())
				this->after_delay(this->counting_frequency, this->count_new_jobs_, 3);
		};
		after_delay(counting_frequency, count_new_jobs_, 3);

		reserve_dispatch_event();

		file_logger = spdlog::basic_logger_mt("file_logger", "logs/log_output_new.txt");
		file_logger->set_pattern("[%l] %v");
		initialise_tp();
	}

	Queue& ClusterSimulation::find_queue(const std::string& name)
	{
		if (name == "-" || name.empty())
			return all_queues_[0];

		auto it = all_queues_.begin();
		it = std::find_if(it, all_queues_.end(),
		             [&name](Queue& queue) { return queue.name == name; });

		if (it == all_queues_.end())
			throw std::out_of_range("Can't find a queue of name " + name + " in this simulation.");

		return *it;
	}

	const Host& ClusterSimulation::find_host(const std::string& name) const
	{
		auto it = cluster_.find_node(name);
		//if (it == cluster_.end())
		//	throw std::out_of_range("Can't find a host of name " + name + " in this cluster.");

		return *(it->second);
	}

	ClusterSimulation::EventItem::EventItem(const ScenarioEntry& entry, ClusterSimulation& simulation)
	{
		priority = 0;
		time = entry.timestamp;

		if (entry.type == ScenarioEntry::ScenarioEntryType::SUBMISSION)
		{
			action = [&simulation, entry]
			{
				Queue& queue = simulation.find_queue(entry.event_detail.queue_name);
			
				queue.enqueue(Job{ entry, queue, simulation.get_current_time() });

				//queue.dispatch();
				simulation.reserve_dispatch_event();
				
				simulation.num_submitted_jobs_++;
				simulation.newly_submitted_jobs_++;
				//log(LogLevel::info, "newly_submitted_jobs{0}", simulation.newly_submitted_jobs_);
			};
		}
		else if (entry.type == ScenarioEntry::ScenarioEntryType::CHANGE_STATUS)
		{
			action = [&simulation, &entry]
			{
				if (entry.event_detail.host_name.empty())
					return;

				Host& host = *simulation.get_cluster().find_node(entry.event_detail.host_name)->second;

				host.set_status(entry.event_detail.host_status);
				host.cpu_factor = entry.event_detail.cpu_factor;
				host.ncpus = entry.event_detail.ncpus;
				host.nprocs = entry.event_detail.nprocs;
				host.ncores = entry.event_detail.ncores;
				host.nthreads = entry.event_detail.nthreads;

				Utils::enum_const_ref_holder<HostStatus> test = Utils::enum_to_string<HostStatus>(host.status);
				std::stringstream ss;
				ss << test;

				log(LogLevel::info, "Host {0}'s status is changed to {1}", host.id, ss.str());
			};
		}
		else
		{
			throw std::logic_error("Not implemented;");
		}
	}


	/**
	 * 
	 */
	void ClusterSimulation::after_delay(std::chrono::milliseconds delay, Action block, int priority)
	{
		events_.push(EventItem{current_time_ + delay, std::move(block), priority});
		//spdlog::debug("Event is added ");
	}

	// No use
	bool ClusterSimulation::run(ms run_time)
	{
		//while (!events_.empty() && current_time_ <= run_time)
		//	next();

		std::cout << "Simulation finished. " << std::endl;
		return true;
	}

	bool ClusterSimulation::run()
	{
		std::cout << "Simulation start!" << std::endl;
		
		const int total_count{scenario_.count()};
		int previous_counter{0};
		if constexpr (!console_output)
			//std::cout << "Remaining scenarios:  ";
			std::cout << std::endl;
		while (true)
		{
			if (!scenario_.is_empty())
			{
				std::vector<ScenarioEntry> next_entries;
				ms next_arrival_time;
				std::tie(next_entries, next_arrival_time) = scenario_.pop_all_latest();
				if constexpr(!console_output)
				{
					//std::cout << 
					//	std::string(std::to_string(previous_counter).length(), '\b')
					//	<< scenario_.count();
					//previous_counter = scenario_.count();
					std::cout << "\33[2K\r" << "Remaining scenarios: " << scenario_.count();
				}

				auto next_event_time = events_.top().time;
				while (next_event_time <= next_arrival_time)
				{
					next();
					next_event_time = events_.top().time;
				}

				for (const auto& entry : next_entries)
				{
					events_.push(EventItem(entry, *this));
				}
			}
			else
			{
				while (!events_.empty())
					next();
				break;	
			}
			
		}
		return true;
	}

	void ClusterSimulation::print_summary() const
	{
		auto total_simulation_time = latest_finish_time_.time_since_epoch().count();
		//double total_cpu_power{ 0.0 };
		int num_total_available_hosts{ 0 };
		int num_total_applications{ scenario_.num_unique_apps() };
		for (const auto& host : cluster_.vector())
		{
			if (!host.is_available_at_least_once)
				continue;

			num_total_available_hosts++;
			//total_cpu_power += 
		}

		std::string algorithm_name;
		if (const auto algorithm = all_queues_.front().current_algorithm; algorithm != nullptr)
			algorithm_name = algorithm->get_name();
		else
			algorithm_name = "Scenario";

		std::stringstream ss;

		ss << "\n" <<
			"## Simulation Summary" << "\n" <<
			"### Simulated duration (MakeSpan): " << total_simulation_time << " ms\n" <<
			"### Available hosts: " << num_total_available_hosts << "\n" <<
			"### Number of applications: " << num_total_applications << "\n" <<
			"### Number of submitted jobs: " << num_submitted_jobs_ << "\n" <<
			"### Number of failed jobs: " << num_failed_jobs_ << "\n" <<
			"### Total pending duration: " << total_pending_duration_.count() << " ms" << "\n" <<
			"#### Queue algorithm: " << algorithm_name << "\n" <<
			//"### Number of successful jobs: " << num_successful_jobs_ << "\n" <<
			"#### Dispatch frequency: " << dispatch_frequency.count() << " ms\n" <<
			"#### Logging frequency: " << logging_frequency.count() << " ms\n" <<
			std::endl;


		if constexpr(console_output)
			log(LogLevel::info, ss.str());
		else
			std::cout << ss.str();
		

		performance_<< "MakeSpan\n" <<
			"### Simulated duration: " << total_simulation_time << "\n" <<
			"### Number of submitted jobs: " << num_submitted_jobs_ << "\n" <<	
			std::endl;
		
		
		performance_ << " end : "<< using_slot_record_.size() << "\n";
		
		std::vector<std::pair<ms, std::size_t>> records(using_slot_record_.begin(), using_slot_record_.end());
		std::sort(records.begin(), records.end());

		for (auto [time, count] : records)
			performance_ << " time : "<< time.time_since_epoch().count() << ", value : "<< count << "\n";

		records = std::vector<std::pair<ms, std::size_t>>(job_submit_record_.begin(), job_submit_record_.end());
		std::sort(records.begin(), records.end());
		
		for (auto [time, count] : job_submit_record_)
			job_submit_<< " time : "<< time.time_since_epoch().count() << ", value : "<< count << "\n";

		for (auto [time, count] : pending_record_)
			pending_jobs_file << time.time_since_epoch().count() << ", " << count << "\n";
	}

	void ClusterSimulation::next()
	{
		const auto event_item = events_.top();
		events_.pop(); 

		if (current_time_ != event_item.time /*&& event_item.priority < 2*/)
		{
			current_time_ = event_item.time;
			if (event_item.priority < 2)
			{
				log(LogLevel::info, "Current Time: {0}", current_time_.time_since_epoch().count());
				update_latest_finish_time(current_time_);
			}
		}
		event_item.action();

		//log(LogLevel::warn, "Time: {0}, Priority: {1}", event_item.time.time_since_epoch().count(), event_item.priority);
	}

	void ClusterSimulation::reserve_dispatch_event()
	{
		if (next_dispatch_reserved)
			return;

		after_delay(
			Utils::get_time_left_until_next_period(current_time_, dispatch_frequency),
			std::ref(dispatcher_), 1);

		next_dispatch_reserved = true;
	}

	/**
	 * Initialise the table header of jobmart_raw_replica file.
	 */
	void ClusterSimulation::initialise_tp()
	{
		if constexpr (!jobmart_file_output)
			return;

		jobmart_file_.open("logs/jobmart_raw_replica.txt");
		//tp_.AddColumn("submit_time_gmt", 10);
		//tp_.AddColumn("start_time_gmt", 10);
		//tp_.AddColumn("finish_time_gmt", 10);
		//tp_.AddColumn("submit_time", 10);
		tp_.AddColumn("start_time", 10);
		tp_.AddColumn("finish_time", 10);
		tp_.AddColumn("queue_name", 20);
		tp_.AddColumn("exec_hostname", 10);
		tp_.AddColumn("num_exec_procs", 10);
		tp_.AddColumn("num_slots", 10);
		tp_.AddColumn("job_exit_status", 10);
		tp_.AddColumn("application_name", 20);
		tp_.AddColumn("job_id", 5);
		tp_.AddColumn("job_pend_time", 10);
		tp_.AddColumn("job_run_time", 10);
		tp_.AddColumn("job_mem_usage", 10);
		tp_.AddColumn("job_swap_usage", 10);
		tp_.AddColumn("job_cpu_time", 10);
		tp_.PrintHeader();
	}
}

