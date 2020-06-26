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

namespace cs
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

		if constexpr (config::USE_ONLY_DEFAULT_QUEUE)
		{
			all_queues_.clear();
			all_queues_.emplace_back(*this);
		}
		else
			std::sort(all_queues_.begin(), all_queues_.end());

		if constexpr (config::JOB_SUBMIT_FILE_OUTPUT)
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
					this->after_delay(config::COUNTING_FREQUENCY, this->count_new_jobs_, 3, EventItem::Type::LOG);
			};
			after_delay(config::COUNTING_FREQUENCY, count_new_jobs_, 3, EventItem::Type::LOG);
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

	ClusterSimulation::ClusterSimulation(Cluster& cluster, Scenario& scenario, std::string_view binary_allocation_file_path, bool minimal_format)
	: cluster_{cluster}
	, scenario_{scenario}
	, dispatcher_{this}
	{
		cluster_.set_simulation(this);
		all_queues_.emplace_back(*this);
		auto& queue = all_queues_.back();
		
		// Open the binary file
		std::ifstream file(binary_allocation_file_path.data(), std::ios::binary | std::ios::ate);
		if (!file) throw std::runtime_error("Can't find a binary allocation file.");

		file.seekg(0, std::ifstream::end);
		const std::size_t size = file.tellg();
		file.seekg(0, std::ifstream::beg);
		if (size == 0) throw std::runtime_error("Binary allocation file is empty.");

		std::cout << "Parsing allcation binary..." << std::endl;

		auto buffer = std::make_unique<std::byte[]>(size);

		file.read(reinterpret_cast<char*>(buffer.get()), size);
		std::size_t read = 0;
		//char* ptr = reinterpret_cast<char*>(buffer.get());
		auto* ptr = buffer.get();

		struct MinimalEntry
		{
			uint16_t allocated_host_id;
		};

		if (minimal_format)
		{
			auto* array = reinterpret_cast<MinimalEntry*>(ptr);
			const std::size_t length = size / sizeof(MinimalEntry);
			if (length != scenario.count()) throw std::runtime_error("Invalid allocation binary for the given scenario.");

			for (auto i = 0; i < length; ++i)
			{
				auto& host = cluster[array[i].allocated_host_id];

				auto entry = scenario.pop();
				ms current = entry.timestamp;
				current_time_ = current;
				auto job = std::make_shared<Job>(std::move(entry), queue, current);

				auto ready_duration = host.get_ready_duration(*job);
				if (ready_duration > 0ms)
					host.execute_job_when_ready(job, ready_duration);
				else
					host.execute_job(job);
			}
			current_time_ = scenario.initial_time_point;
		}
		else
			while (read < size)
			{
				
			}
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

	bool ClusterSimulation::run()
	{
		std::cout << "Simulation start!" << std::endl;
		const auto start_timer = high_resolution_clock::now();

		if constexpr(!config::CONSOLE_OUTPUT)
			std::cout << std::endl;
		
		while (true)
		{
			if (!scenario_.is_empty())
			{
				auto next_entries = scenario_.pop_all_latest();
				auto next_arrival_time = next_entries.front().timestamp;
				
				if constexpr(!config::CONSOLE_OUTPUT)
					std::cout << "\33[2K\r" << "Remaining scenarios: " << scenario_.count();

				auto next_event_time = events_.top().time;
				while (next_event_time <= next_arrival_time)
				{
					next();
					next_event_time = events_.top().time;
				}

				for (auto& entry : next_entries)
					events_.push(EventItem(std::move(entry), *this));
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
		const auto total_simulation_time = latest_finish_time_.time_since_epoch().count();
		//double total_cpu_power{ 0.0 };
		std::size_t num_total_available_hosts{ 0 };
		const std::size_t num_total_applications{ scenario_.num_unique_apps() };
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
			"### Total Queuing Time :" << total_queuing_time_.count() << " ms\n" <<
			"### Available hosts: " << num_total_available_hosts << "\n" <<
			"### Number of queues: " << all_queues_.size() << "\n" <<
			"### Number of applications: " << num_total_applications << "\n" <<
			"### Number of submitted jobs: " << num_submitted_jobs_ << "\n" <<
			"### Number of failed jobs: " << num_failed_jobs_ << "\n" <<
			"### Total pending duration: " << total_pending_duration_.count() << " ms\n" <<
			"#### Queue algorithm: " << algorithm_name << "\n" <<
			//"### Number of successful jobs: " << num_successful_jobs_ << "\n" <<
			"#### Dispatch frequency: " << config::DISPATCH_FREQUENCY.count() << " ms\n" <<
			"#### Logging frequency: " << config::LOGGING_FREQUENCY.count() << " ms\n" <<
			"#### Runtime multiplier: " << config::RUNTIME_MULTIPLIER << "\n" <<
			"#### Actual run time of simulation: " << duration_cast<seconds>(actual_run_time_).count() << " s"
			<< std::endl;


		if constexpr(config::CONSOLE_OUTPUT)
			log(LogLevel::info, ss.str());
		else
			std::cout << ss.str();
		

		performance_file_ << ss.str() << std::endl;
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


		if (config::LOG_ALLOCATION)
		{
			generate_allocation_binary(std::filesystem::path(config::LOG_DIRECTORY) / config::ALLOCATION_BINARY_FILE_NAME);
		}
	}

	void ClusterSimulation::next()
	{
		const auto event_item = events_.top();
		events_.pop();

		if constexpr (config::DEBUG_EVENTS)
		{
			log(LogLevel::debug, "Event [{0}] Time: {1}ms Priority: {2}", 
				EventItem::type_strings[static_cast<int>(event_item.type)], event_item.time.time_since_epoch().count(), event_item.priority);
		}
		

		if (current_time_ != event_item.time /*&& event_item.priority < 2*/)
		{
			if (current_time_ > event_item.time)
			{
				int i = 0;
			}
			
			current_time_ = event_item.time;
			if (event_item.priority < 2)
			{
				if constexpr (LOG_ANY)
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
			Utils::get_time_left_until_next_period(current_time_, config::DISPATCH_FREQUENCY),
			std::ref(dispatcher_), 1, EventItem::Type::DISPATCH);


		next_dispatch_reserved = true;
	}

	/**
	 * Initialise the table header of jobmart_raw_replica file.
	 */
	void ClusterSimulation::initialise_tp()
	{
		if constexpr (!config::JOBMART_FILE_OUTPUT)
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

