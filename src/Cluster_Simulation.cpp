#include <iostream>
#include <sstream>
#include "../includes/Queue.h"
#include "../includes/Cluster.h"
#include "../includes/Scenario.h"
#include "../includes/Cluster_Simulation.h"
#include "../includes/Job.h"
#include "../dependencies/spdlog/spdlog.h"

//#include "spdlog/spdlog.h"
#include "../dependencies/spdlog/sinks/basic_file_sink.h"
#include "../dependencies/spdlog/common.h"
#include <fstream>

//#include "dependencies/

namespace ClusterSimulator
{
	std::shared_ptr<spdlog::logger> ClusterSimulation::file_logger = spdlog::basic_logger_mt("file_logger", "log_output.txt");
	std::ofstream ClusterSimulation::jobmart_file_;
	bprinter::TablePrinter ClusterSimulation::tp_{ &jobmart_file_ };

	ClusterSimulation::ClusterSimulation(Scenario& scenario, Cluster& cluster)
		: cluster_{ cluster }, scenario_{ scenario }, all_queues_{ scenario.generate_queues(*this) }
	{
		// TODO: set default queue

		current_time_ = scenario.initial_time_point;
		std::sort(all_queues_.begin(), all_queues_.end());

		dispatch_all_ = [this]
		{
			for (auto& q : this->all_queues_)
				q.dispatch();
			this->after_delay(this->dispatch_frequency, this->dispatch_all_, true);
		};

		//spdlog::register_logger(file_logger);
		//spdlog::

		initialise_tp();
		std::cout << "Simulation start!" << std::endl;
	}

	Queue& ClusterSimulation::find_queue(const std::string& name)
	{
		auto it = all_queues_.begin();
		it = std::find_if(it, all_queues_.end(),
		             [&name](Queue& queue) { return queue.name == name; });

		if (it == all_queues_.end())
			throw std::out_of_range("Can't find a queue of name " + name + " in this simulation.");

		return *it;
	}

	Host& ClusterSimulation::find_host(const std::string& name) const
	{
		auto it = std::find_if(cluster_.begin(), cluster_.end(),
			[&name](Host& host) { return host.name == name; });

		if (it == cluster_.end())
			throw std::out_of_range("Can't find a host of name " + name + " in this cluster.");

		return *it;
	}

	ClusterSimulation::EventItem::EventItem(const ScenarioEntry& entry, ClusterSimulation& simulation)
	{
		time = entry.timestamp;

		if (entry.type == ScenarioEntry::ScenarioEntryType::SUBMISSION)
		{
			action = [&simulation, &entry]
			{
				if (entry.event_detail.queue_name == "-" || entry.event_detail.queue_name.empty())
					return;

				Queue& queue = simulation.find_queue(entry.event_detail.queue_name);
			
				queue.enqueue(Job{ entry, queue, simulation.get_current_time() });
		
				queue.dispatch();
				
				simulation.num_submitted_jobs_++;
			};
		}
		else if (entry.type == ScenarioEntry::ScenarioEntryType::CHANGE_STATUS)
		{
			action = [&simulation, &entry]
			{
				if (entry.event_detail.host_name.empty())
					return;

				Host& host = simulation.find_host(entry.event_detail.host_name);

				host.set_status(entry.event_detail.host_status);

				Utils::enum_const_ref_holder<HostStatus> test = Utils::enum_to_string<HostStatus>(host.status);
				std::stringstream ss;
				ss << test;

				spdlog::info("Host {0}'s status is changed to {1}", host.id, ss.str());
				file_logger->info("Host {0}'s status is changed to {1}", host.id, ss.str());
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
	void ClusterSimulation::after_delay(std::chrono::milliseconds delay, Action block, bool ignore_timestamp)
	{
		events_.push(EventItem{current_time_ + delay, std::move(block), ignore_timestamp});
		spdlog::debug("Event is added ");
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
		auto entry = scenario_.pop();

		events_.push(EventItem(entry, *this));

		// TODO: periodic dispatching
		//after_delay(dispatch_frequency, dispatch_all_, true);

		//events_.push()
		while (true)
		{
			if (!scenario_.is_empty())
			{
				auto next_entry = scenario_.pop();
				auto next_arrival_time = next_entry.timestamp;


				auto next_event = events_.top();
				while (next_event.time <= next_arrival_time)
				{
					next();
					next_event = events_.top();
				}

				events_.push(EventItem(next_entry, *this));
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
		auto total_simulation_time = current_time_.time_since_epoch().count();
		//double total_cpu_power{ 0.0 };
		int num_total_available_hosts{ 0 };
		int num_total_applications{ scenario_.num_unique_apps() };
		for (const auto& host : cluster_)
		{
			if (!host.is_available_at_least_once)
				continue;

			num_total_available_hosts++;
			//total_cpu_power += 
		}

		std::stringstream ss;

		ss << "\n" <<
			"## Simulation Summary" << "\n" <<
			"### Simulated duration: " << total_simulation_time << "\n" <<
			"### Available hosts: " << num_total_available_hosts << "\n" <<
			"### Number of applications: " << num_total_applications << "\n" <<
			"### Number of submitted jobs: " << num_submitted_jobs_ << "\n" <<
			//"### Number of successful jobs: " << num_successful_jobs_ << "\n" <<
			std::endl;


		spdlog::info(ss.str());
		file_logger->info(ss.str());
	}

	void ClusterSimulation::next()
	{
		const auto event_item = events_.top();
		events_.pop();

		if (current_time_ != event_item.time)
		{
			current_time_ = event_item.time;
			if (!event_item.ignore_timestamp)
				log(LogLevel::info, "Current Time: {0}", current_time_.time_since_epoch().count());
		}
		event_item.action();
	}


	void ClusterSimulation::initialise_tp()
	{
		jobmart_file_.open("jobmart_raw_replica.txt");
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
		tp_.AddColumn("job_run_time", 10);
		tp_.AddColumn("job_mem_usage", 10);
		tp_.AddColumn("job_swap_usage", 10);
		tp_.AddColumn("job_cpu_time", 10);
		tp_.PrintHeader();
	}
}

