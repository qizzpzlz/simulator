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
//#include "dependencies/

namespace ClusterSimulator
{
	std::shared_ptr<spdlog::logger> ClusterSimulation::file_logger = spdlog::basic_logger_mt("file_logger", "log_output.txt");

	ClusterSimulation::ClusterSimulation(Scenario& scenario, Cluster& cluster)
		: cluster_{ cluster }, scenario_{ scenario }, all_queues_{ scenario.generate_queues(*this) }
	{
		// TODO: set default queue

		current_time_ = scenario.initial_time_point;

		//spdlog::register_logger(file_logger);
		//spdlog::

		std::cout << "Simulation start!" << std::endl;
	}

	ClusterSimulation::~ClusterSimulation()
	{
	}

	Queue& ClusterSimulation::find_queue(const std::string& name)
	{
		auto it = std::find_if(all_queues_.begin(), all_queues_.end(),
		             [&name](Queue& queue) { return queue.name == name; });

		if (it == all_queues_.end())
			throw std::out_of_range("Can't find a queue of name " + name + " in this simulation.");

		return *it;
	}

	std::vector<Host> ClusterSimulation::all_host()
	{	
		auto it = cluster_.get_all_nodes();
		spdlog::info("size {}", it.size());
		return it;
		
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
			
				queue.enqueue(Job{ entry, queue });
		
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
	void ClusterSimulation::after_delay(std::chrono::milliseconds delay, Action block)
	{
		events_.push(EventItem{current_time_ + delay, std::move(block)});
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
		current_time_ = event_item.time;
		spdlog::info("Current Time: {0}", current_time_.time_since_epoch().count());
		file_logger->info("Current Time: {0}", current_time_.time_since_epoch().count());
		event_item.action();
	}
}

