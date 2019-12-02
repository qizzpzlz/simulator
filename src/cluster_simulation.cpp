#include <iostream>
#include <sstream>
#include "queue.h"
#include "cluster.h"
#include "scenario.h"
#include "cluster_simulation.h"
#include "job.h"
#include "queue_algorithm.h"

#include <fstream>

//#include "dependencies/

namespace ClusterSimulator
{
	ClusterSimulation::ClusterSimulation(Scenario& scenario, Cluster& cluster, const QueueAlgorithm& algorithm)
		: cluster_{ cluster }
		, scenario_{ scenario }
		, all_queues_{ scenario.generate_queues(*this) }
		, dispatcher_{ this }
	{
		cluster.set_simulation(this);

		// TODO: set default queue

		current_time_ = scenario.initial_time_point;
		std::sort(all_queues_.begin(), all_queues_.end());

		if constexpr (JOB_SUBMIT_FILE_OUTPUT)
		{
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
		}
		
		//set algoritms
		for(auto& q : this->all_queues_)
		{
			q.set_algorithm(&algorithm);
			std::cout << q.name << " 's algorithm is set to " << algorithm.get_name() << std::endl;
		}


		reserve_dispatch_event();

		initialise_tp();
	}

	Queue& ClusterSimulation::find_queue(const std::string& name)
	{
		auto it = all_queues_.begin();
		it = std::find_if(it, all_queues_.end(),
		             [&name](const Queue& queue) { return queue.name == name; });

		if (it == all_queues_.end())
			throw std::out_of_range("Can't find a queue of name " + name + " in this simulation.");

		return *it;
	}

	//const Host& ClusterSimulation::find_host(const std::string& name) const
	//{
	//	auto it = cluster_.find_node(name);
	//	if (it == cluster_.end())
	//		throw std::out_of_range("Can't find a host of name " + name + " in this cluster.");

	//	return it->second;
	//}

	ClusterSimulation::EventItem::EventItem(const ScenarioEntry& entry, ClusterSimulation& simulation)
	{
		priority = 0;
		time = entry.timestamp;

		if (entry.type == ScenarioEntry::ScenarioEntryType::SUBMISSION)
		{
			action = [&simulation, entry]
			{
				if (entry.event_detail.queue_name == "-" || entry.event_detail.queue_name.empty())
					return;

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
				//if (entry.event_detail.host_name.empty())
				//	return;

				//Host& host = simulation.get_cluster().find_node(entry.event_detail.host_name)->second;

				//host.set_status(entry.event_detail.host_status);
				//host.cpu_factor = entry.event_detail.cpu_factor;
				//host.ncpus = entry.event_detail.ncpus;
				//host.nprocs = entry.event_detail.nprocs;
				//host.ncores = entry.event_detail.ncores;
				//host.nthreads = entry.event_detail.nthreads;

				//Utils::enum_const_ref_holder<HostStatus> test = Utils::enum_to_string<HostStatus>(host.status);
				//std::stringstream ss;
				//ss << test;

				//log(LogLevel::info, "Host {0}'s status is changed to {1}", host.id, ss.str());
			};
		}
		else
		{
			throw std::logic_error("Not implemented;");
		}
	}


	/**
	 * Reserve an event after a specified duration.
	 */
	void ClusterSimulation::after_delay(milliseconds delay, Action block, int priority)
	{
		events_.push(EventItem{current_time_ + delay, std::move(block), priority});
	}

	bool ClusterSimulation::run()
	{
		std::cout << "Simulation start!" << std::endl;
		const auto start_timer = high_resolution_clock::now();

		if constexpr(!CONSOLE_OUTPUT)
			std::cout << std::endl;
		
		while (true)
		{
			if (!scenario_.is_empty())
			{
				auto [next_entries, next_arrival_time] = scenario_.pop_all_latest();
				
				if constexpr(!CONSOLE_OUTPUT)
					std::cout << "\33[2K\r" << "Remaining scenarios: " << scenario_.count();

				auto next_event_time = events_.top().time;
				while (next_event_time <= next_arrival_time)
				{
					next();
					next_event_time = events_.top().time;
				}

				for (const auto& entry : next_entries)
					events_.push(EventItem(entry, *this));
			}
			else
			{
				while (!events_.empty())
					next();
				break;	
			}
		}

		const auto finish_timer = high_resolution_clock::now();
		actual_run_time_ = finish_timer - start_timer;
		return true;
	}

	void ClusterSimulation::print_summary()
	{
		auto total_simulation_time = latest_finish_time_.time_since_epoch().count();
		//double total_cpu_power{ 0.0 };
		size_t num_total_available_hosts{ 0 };
		size_t num_total_applications{ scenario_.num_unique_apps() };
		//for (const auto& [name, host] : cluster_)
		for (auto& host : cluster_)
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
			"### Total pending duration: " << total_pending_duration_.count() << " ms\n" <<
			"#### Queue algorithm: " << algorithm_name << "\n" <<
			//"### Number of successful jobs: " << num_successful_jobs_ << "\n" <<
			"#### Dispatch frequency: " << dispatch_frequency.count() << " ms\n" <<
			"#### Logging frequency: " << logging_frequency.count() << " ms\n" <<
			"#### Actual run time of simulation: " << duration_cast<seconds>(actual_run_time_).count() << " s"
			<< std::endl;


		if constexpr(CONSOLE_OUTPUT)
			log(LogLevel::info, ss.str());
		else
			std::cout << ss.str();
		

		performance_file_ << "MakeSpan\n" <<
			"### Simulated duration: " << total_simulation_time << "\n" <<
			"### Number of submitted jobs: " << num_submitted_jobs_ << "\n" <<	
			std::endl;
		
		
		performance_file_ << " end : "<< using_slot_record_.size() << "\n";
		// for (const slot_record_entry s : using_slot_record_) performance_ << " time : "<< s.time_stamp.time_since_epoch().count() << ", value : "<< s.value << "\n";
		
		std::vector<std::pair<ms ,int>> records(using_slot_record_.begin(), using_slot_record_.end());
		std::sort(records.begin(), records.end());

		for (auto [time, count] : records)
			performance_file_ << " time : "<< time.time_since_epoch().count() << ", value : "<< count << "\n";

		records = std::vector<std::pair<ms, int>>(job_submit_record_.begin(), job_submit_record_.end());
		std::sort(records.begin(), records.end());
		
		for (auto [time, count] : job_submit_record_)
			job_submit_file_ << " time : "<< time.time_since_epoch().count() << ", value : "<< count << "\n";

		for (auto [time, count] : pending_record_)
			pending_jobs_file_ << time.time_since_epoch().count() << ", " << count << "\n";
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
				if constexpr (ClusterSimulation::LOG_ANY)
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
		if constexpr (!JOBMART_FILE_OUTPUT)
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
		// tp_.AddColumn("num_exec_procs", 10);
		tp_.AddColumn("num_slots", 10);
		// tp_.AddColumn("job_exit_status", 10);
		// tp_.AddColumn("application_name", 20);
		tp_.AddColumn("job_id", 5);
		tp_.AddColumn("job_pend_time", 10);
		tp_.AddColumn("job_run_time", 10);
		// tp_.AddColumn("job_mem_usage", 10);
		// tp_.AddColumn("job_swap_usage", 10);
		// tp_.AddColumn("job_cpu_time", 10);
		tp_.PrintHeader();
	}
}

