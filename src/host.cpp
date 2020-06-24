#include "job.h"
#include "host.h"
#include "cluster_simulation.h"
#include "enum_converter.h"

namespace ClusterSimulator
{
	int Host::id_gen_ = 0;
	std::random_device Host::rd_{};
	std::mt19937 Host::gen_(rd_());
	std::uniform_int_distribution<> Host::dist_(1, MAX_RAND_NUM);

	/**
	 * Estimate the duration to wait for a specified job to be executed on this host.
	 */
	milliseconds Host::get_ready_duration(const Job& job) const
	{
		auto remaining = remaining_slots();
		const auto slot_required = job.slot_required;

		//if (slot_required <= remaining) return 0ms;

		if (!reserved_jobs_.empty())
		{
			// Check the job can be executed right away
			if (slot_required <= remaining)
			{
				// Jobs reserved before the expected finish time of the job
				// should be rearranged.
				const auto finish_time = simulation->current_time_ + get_expected_run_duration(job);
				auto rearrangement_delay = 0ms;
				for (auto& [ptr, id] : reserved_jobs_)
				{
					const auto future = ptr->start_time;
					if (future < finish_time)
					{
						auto future_remaining = remaining;
						for (auto& running_job : running_jobs_)
							if (running_job->finish_time <= future)
								future_remaining += running_job->slot_required;
						for (const auto& reserved_job : reserved_jobs_)
							if (reserved_job.first->start_time <= future &&
								reserved_job.first->finish_time >= future)	// a reserved job is running at the future
								future_remaining -= reserved_job.first->slot_required;

						if (future_remaining < ptr->slot_required)
							rearrangement_delay += finish_time - ptr->start_time;
					}
				}
				return rearrangement_delay;
			}

			// Get the earliest reserved job on this host.
			const auto it = std::min_element(reserved_jobs_.cbegin(), reserved_jobs_.cend(),
				[](const std::pair<std::shared_ptr<Job>, std::size_t>& a, const std::pair<std::shared_ptr<Job>, std::size_t>& b)
				{ return a.first->start_time < b.first->start_time; });
			const auto time_reserved_jobs_start = (*it).first->start_time;

			// Get the remaining slots and jobs at the time when
			// the earliest reserved job is about to be executed.
			int future_remaining = remaining;
			std::vector<std::shared_ptr<Job>> future_jobs{};
			for (auto& [ptr, id] : reserved_jobs_)
				future_jobs.emplace_back(ptr);
			for (auto& reserved_job : future_jobs)
				future_remaining -= reserved_job->slot_required;
			for (auto& running_job : running_jobs_)
				if (time_reserved_jobs_start >= running_job->finish_time)
					future_remaining += running_job->slot_required;
				else
					future_jobs.push_back(running_job);

			std::sort(future_jobs.begin(), future_jobs.end(), [](std::shared_ptr<Job>& a, std::shared_ptr<Job>& b)
				{
					return a->finish_time < b->finish_time;
				});

			for (auto& future_job : future_jobs)
			{
				future_remaining += future_job->slot_required;
				if (slot_required <= future_remaining)
					return future_job->finish_time - simulation->get_current_time();
			}
		}
		else if (slot_required <= remaining) return 0ms;

		
		// Running jobs are sorted with it's finish time.
		// Therefore we need to iterate the list in reversed order
		// so that we can get expected available slots at future times in the correct order.
		for (auto it = running_jobs_.rbegin(); it != running_jobs_.rend(); ++it)
		{
			auto& running_job = **it;
			remaining += running_job.slot_required;
			if (slot_required <= remaining)
			{
				return running_job.finish_time - simulation->get_current_time() + 1ms;
			}
		}

		throw std::runtime_error("Can't get ready duration.");
	}

	/**
	 * Estimate the expected completion time of a specified job on this host.
	 * Expected completition time is the sum of expected execution time of the job
	 * and the availability time of this host after completing previously assigned jobs.
	 */
	milliseconds Host::get_expected_completion_duration(const Job& job) const
	{
		if constexpr (!ClusterSimulation::USE_STATIC_HOST_TABLE_FOR_JOBS)
			if (!is_compatible(job)) return milliseconds::max();	// Returns infinity if it can't be run.

		return get_expected_run_duration(job) + get_ready_duration(job);
	}

	/**
	 * Estimate the expected run time of a specified job on this host.
	 * A job's run time is determined by the sum of the job's cpu_time multiplied
	 * by the running host's cpu_factor and the job's non_cpu_time.
	 * (job.run_time = job.cpu_time / host.cpu_factor + job.non_cpu_time)
	 */
	milliseconds Host::get_expected_run_duration(const Job& job) const noexcept
	{
		// Estimated run time: cpu_time / factor + non_cpu_time
		return duration_cast<milliseconds>(
			(job.cpu_time / cpu_factor + job.non_cpu_time) * ClusterSimulation::RUNTIME_MULTIPLIER);
	}

	/**
	 * Execute a specified job in this host.
	 * This will cause job finished event after the estimated runtime for
	 * running the job in this host.
	 * The runtime is calculated by: job.cpu_time / host.cpu_factor + job.non_cpu_time .
	 * **The ownership of the job is transferred to this host.
	 */
	void Host::execute_job(std::shared_ptr<Job>& job_ptr)
	{
		bool should_check_reserved_jobs{ false };
		if (!reserved_jobs_.empty())
		{
			auto it = reserved_jobs_.begin();
			while (it != reserved_jobs_.end())
			{
				if (it->first == job_ptr)
					break;
				++it;
			}

			
			if (it != reserved_jobs_.end())
				reserved_jobs_.erase(it);
			else
				should_check_reserved_jobs = true;
		}
		
		Job& job{ *job_ptr };

		// Update dispatched slots
		simulation->num_dispatched_slots += job.slot_required;
		if (job.total_pending_duration > 0ms)
			simulation->update_pending_duration(job.total_pending_duration);

		job.queue_managing_this_job->using_job_slots += job.slot_required;

		if constexpr (ClusterSimulation::LOG_ANY)
			simulation->log(LogLevel::info, "Job #{0} is executed in Host #{1}"
				, job.id, this->id);

		// Estimate run time for the job in this host.
		const auto run_time{ get_expected_run_duration(job) };
		if (run_time < 0ms)
			throw std::runtime_error("run time can't be negative.");

		try_update_expected_time_of_completion(run_time);

		if constexpr (ClusterSimulation::LOG_ANY)
			if (num_current_running_slots + job.slot_required > max_slot)
				simulation->log(LogLevel::err, 
					"Host #{0}: Slot required for job {1} cannot be fulfilled with this host.", id, job.id);

		const ms start_time{ simulation->get_current_time() };
		job.start_time = start_time;
		job.finish_time = start_time + run_time;
		job.set_run_host_name(name_);
		job.state = JobState::RUN;


		if (should_check_reserved_jobs)
		{
			//const auto min_it = std::min_element(reserved_jobs_.cbegin(), reserved_jobs_.cend(), 
			//	[](const std::pair<std::shared_ptr<Job>, std::size_t>& a, const std::pair<std::shared_ptr<Job>, std::size_t>& b)
			//	{ return a.first->start_time < b.first->start_time; });
			//const auto time_reserved_jobs_start = (*min_it).first->start_time;

			///***
			// * If new job is finished after the latest reserved job,
			// * this could affect the availability of reserved jobs at
			// * the reserved time.
			// * 1. The simplest solution to this problem would be making pending
			// *	  all the reserved jobs. However this could deteriorate the
			// *	  reservation system.
			// * 2. We can delay or recalculate all the reserved jobs.
			// *	  
			// */
			//if (job.finish_time > time_reserved_jobs_start)
			//{
			//	// Implementation of the first solution:
			//	for (auto& [r_job, id] : reserved_jobs_)
			//	{
			//		r_job->queue_managing_this_job->add_pending_job(r_job);
			//		simulation->erase_event(id);
			//	}

			//	reserved_jobs_.clear();
			//}
		}
		

		// Calculate Queuing Time here.
		auto q_time = start_time - job.submit_time;
		simulation->update_total_queuing_time(q_time);

		// Reserve job finished event.
		simulation->after_delay(run_time, std::bind(&Host::exit_job, this),
			0, ClusterSimulation::EventItem::Type::JOB_FINISHED);

		cluster->update();

		// Register job to this host
		num_current_running_slots += job.slot_required;
		running_jobs_.push_back(job_ptr);
		// Sort running_jobs_ here; ordering depends on finish time of job
		std::sort(running_jobs_.begin(), running_jobs_.end(),
			[](std::shared_ptr<Job>& a, std::shared_ptr<Job>& b)
			{
				return a->finish_time > b->finish_time;
			});

		// Record allocation
		simulation->log_allocation(job, id);
	}

	/**
	 * TODO: Comment
	 */
	void Host::execute_job_when_ready(std::shared_ptr<Job>& job_ptr, milliseconds delay)
	{
		Job& job = *job_ptr;

		const auto remaining = remaining_slots();
		if (job.slot_required <= remaining)
		{
			execute_job(job_ptr);

			//auto rearrangement_delay = 0ms;
			for (auto& [ptr, id] : reserved_jobs_)
			{
				const auto future = ptr->start_time;
				if (future < job.finish_time)
				{
					auto future_remaining = remaining;
					for (auto& running_job : running_jobs_)
						if (running_job->finish_time <= future)
							future_remaining += running_job->slot_required;
					for (const auto& reserved_job : reserved_jobs_)
						if (reserved_job.first->start_time <= future &&
							reserved_job.first->finish_time >= future)	// a reserved job is running at the future
							future_remaining -= reserved_job.first->slot_required;

					if (future_remaining < ptr->slot_required)
					{
						const auto delay = job.finish_time - ptr->start_time;
						ptr->start_time += delay;
						ptr->finish_time += delay;
						simulation->add_delay(id, delay);
					}
				}
			}
			
			return;
		}
		
		//const auto delay = get_ready_duration(job);
		const auto run_time{ duration_cast<milliseconds>(get_expected_run_duration(job) * ClusterSimulation::RUNTIME_MULTIPLIER) };
		
		job.start_time = simulation->get_current_time() + delay;
		job.finish_time = job.start_time + run_time;
		job.set_run_host_name(name_);
		job.state = JobState::WAIT;
		
		//std::sort(reserved_jobs_.begin(), reserved_jobs_.end(),
		//	[](std::shared_ptr<Job>& a, std::shared_ptr<Job>& b)
		//	{
		//		return a->finish_time > b->finish_time;
		//	});
		auto id = simulation->after_delay(delay, std::bind(&Host::execute_job, this, job_ptr),
			1, ClusterSimulation::EventItem::Type::JOB_RESERVED);

		reserved_jobs_.emplace_back(job_ptr, id);
		
		if constexpr (ClusterSimulation::LOG_ANY)
			simulation->log(LogLevel::info, "Job #{0} is reserved in Host #{1}", job.id, this->id);
	}

	/**
	 * Complete the job with the nearest finish time among
	 * running jobs in this host.
	 * This method is only used as reserved event for event simulator.
	 */
	void Host::exit_job()
	{
		Job& job{ *running_jobs_.back() };

		if (job.finish_time != simulation->get_current_time())
			throw std::runtime_error("Incongruous Job completion time.");

		//Queue::HostInfo info;
		// if (!job.queue_managing_this_job->try_get_dispatched_host_info(*this, &info))
		// 	throw std::runtime_error("Queue managing a job does not have information about the host of the job.");

		//info.slot_dispatched -= job.slot_required;
		num_current_running_slots -=  job.slot_required;


		// std::stringstream ss(job.get_exit_host_status());
		// ss >> Utils::enum_from_string<HostStatus>(this->status);

		if constexpr (ClusterSimulation::LOG_ANY)
			simulation->log(LogLevel::info,
				"Job #{0} is finished in Host #{1}. (actual run time: {2} ms, scenario run time: {3} ms)"
				, job.id, this->id, (job.finish_time - job.start_time).count()
				, job.cpu_time.count() + job.non_cpu_time.count());

		cluster->update();

		simulation->num_dispatched_slots -= job.slot_required;
		job.queue_managing_this_job->using_job_slots -= job.slot_required;

		simulation->log_using_slots();
		simulation->log_jobmart(job);

		running_jobs_.pop_back();
	}

	void Host::update_reserved_jobs(std::shared_ptr<Job>& new_job)
	{
		const auto min_it = std::min_element(reserved_jobs_.cbegin(), reserved_jobs_.cend(),
			[](const std::pair<std::shared_ptr<Job>, std::size_t>& a, const std::pair<std::shared_ptr<Job>, std::size_t>& b)
			{ return a.first->start_time < b.first->start_time; });
		const auto time_reserved_jobs_start = (*min_it).first->start_time;

		if (new_job->finish_time > time_reserved_jobs_start)
		{
			for (auto& [r_job, id] : reserved_jobs_)
			{
				
			}

			reserved_jobs_.clear();
		}
	}

	void Host::try_update_expected_time_of_completion(milliseconds run_time) noexcept
	{
		const ms expected_completion_time = cluster->simulation->get_current_time() + run_time;
		if (expected_completion_time > expected_time_of_completion)
			expected_time_of_completion = expected_completion_time;
	}
}

template<> const std::vector<std::string> Utils::enum_strings<ClusterSimulator::HostStatus>::data = {
	"OK",
	"Closed_Adm",
	"Closed_Busy",
	"Closed_Excl",
	"Closed_cu_Excl",
	"Closed_Full",
	"Closed_LIM",
	"Closed_Lock",
	"Closed_Wind",
	"Closed_EGO",
	"Unavail",
	"Unreach"
};