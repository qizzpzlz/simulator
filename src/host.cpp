#include "job.h"
#include "host.h"
#include "cluster_simulation.h"
#include "enum_converter.h"

namespace cs
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

		
		if (slot_required <= remaining) return 0ms;
		
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
	 * Expected completion time is the sum of expected execution time of the job
	 * and the availability time of this host after completing previously assigned jobs.
	 */
	milliseconds Host::get_expected_completion_duration(const Job& job) const
	{
		if constexpr (!config::USE_STATIC_HOST_TABLE_FOR_JOBS)
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
			(job.cpu_time / cpu_factor + job.non_cpu_time) * config::RUNTIME_MULTIPLIER);
	}

	int Host::get_expected_remaining_slots(const Job& job) const
	{
		int remaining = remaining_slots();
		
		if (job.slot_required <= remaining)
			return remaining - job.slot_required;

		const ms ready_time = get_ready_duration(job) + simulation->get_current_time();

		for (auto it = running_jobs_.rbegin(), end = running_jobs_.rend(); it != end; ++it)
		{
			auto& j = **it;
			if (j.finish_time <= ready_time)
			{
				remaining += j.slot_required;
			}
			else
			{
				break;
			}
		}

		return remaining - job.slot_required;
	}

	/**
	 * Execute a specified job in this host.
	 * This will cause job finished event after the estimated runtime for
	 * running the job in this host.
	 * The runtime is calculated by: job.cpu_time / host.cpu_factor + job.non_cpu_time .
	 * **The ownership of the job is transferred to this host.
	 */
	void Host::execute_job(std::shared_ptr<Job>& job_ptr, bool reserved)
	{
		Job& job{ *job_ptr };

		// Update dispatched slots
		simulation->num_dispatched_slots += job.slot_required;
		if (job.total_pending_duration > 0ms)
			simulation->update_pending_duration(job.total_pending_duration);

		job.queue_managing_this_job->using_job_slots += job.slot_required;

		if constexpr (ClusterSimulation::LOG_ANY)
		{
			simulation->log(LogLevel::info, "Job #{0} is executed in Host #{1}"
				, job.id, this->id);

			if (!reserved && num_current_running_slots + job.slot_required > max_slot)
				simulation->log(LogLevel::err,
					"Host #{0}: Slot required for job {1} cannot be fulfilled with this host.", id, job.id);
		}

		const ms start_time{ simulation->get_current_time() };

		const auto run_time{ get_expected_run_duration(job) };
		if (!reserved)
		{
			// Estimate run time for the job in this host.
			if (run_time < 0ms)
				throw std::runtime_error("run time can't be negative.");

			try_update_expected_time_of_completion(run_time);
			
			job.start_time = start_time;
			job.finish_time = start_time + run_time;
			job.set_run_host_name(name_);

			update_job_list(job_ptr);
		}
		else
		{
			//job.queue_managing_this_job->remove_reserved_job(job_ptr);
			--simulation->num_reserved_jobs_;
			simulation->update_pending_duration(start_time - job.submit_time);
		}

		job.state = JobState::RUN;

		// Calculate Queuing Time here.
		auto q_time = start_time - job.submit_time;
		simulation->update_total_queuing_time(q_time);

		// Reserve job finished event.
		simulation->after_delay(run_time, std::bind(&Host::exit_job, this, job_ptr, reserved),
			0, EventItem::Type::JOB_FINISHED);

		// Record allocation
		if (config::LOG_ALLOCATION)
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
			// Current job's slot_required is met. Execute.
			execute_job(job_ptr);
			return;
		}

		const auto start_time = delay + simulation->get_current_time();

		// Remove jobs that finishes before the execution of the current job.
		// The process is based on the assumption that jobs are reserved in the order of
		// minimal completion time.
		while (!running_jobs_.empty())
		{
			const auto& running_job = *running_jobs_.back();
			if (running_job.finish_time <= start_time)
			{
				num_current_running_slots -= running_job.slot_required;
				running_jobs_.pop_back();
			}
			else
				break;
		}
		
		const auto run_time{ duration_cast<milliseconds>(get_expected_run_duration(job) * config::RUNTIME_MULTIPLIER) };
		try_update_expected_time_of_completion(run_time);
		
		job.start_time = simulation->get_current_time() + delay;
		job.finish_time = job.start_time + run_time;
		job.set_run_host_name(name_);
		job.state = JobState::WAIT;
		
		auto id = simulation->after_delay(delay, std::bind(&Host::execute_job, this, job_ptr, true),
			1, EventItem::Type::JOB_RESERVED);
		
		if constexpr (ClusterSimulation::LOG_ANY)
			simulation->log(LogLevel::info, "Job #{0} is reserved in Host #{1}", job.id, this->id);

		update_job_list(job_ptr);

		//job.queue_managing_this_job->add_pending_job<true>(job_ptr);
		++simulation->num_reserved_jobs_;
	}

	void Host::update_job_list(const std::shared_ptr<Job>& job_ptr)
	{
		cluster->update();
		// Register job to this host
		num_current_running_slots += job_ptr->slot_required;
		running_jobs_.push_back(job_ptr);
		// Sort running_jobs_ here; ordering depends on finish time of job
		std::sort(running_jobs_.begin(), running_jobs_.end(),
			[](std::shared_ptr<Job>& a, std::shared_ptr<Job>& b)
			{
				return a->finish_time > b->finish_time;
			});
	}

	/**
	 * Complete the job with the nearest finish time among
	 * running jobs in this host.
	 * This method is only used as reserved event for event simulator.
	 */
	void Host::exit_job(std::shared_ptr<Job> job_ptr, bool reserved)
	{
		Job& job{ *job_ptr };

		if (job.finish_time != simulation->get_current_time())
			throw std::runtime_error("Incongruous Job completion time.");

		

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

		if (const auto it = std::find(running_jobs_.cbegin(), running_jobs_.cend(), job_ptr);
			it != running_jobs_.end())
		{
			num_current_running_slots -= job.slot_required;
			running_jobs_.erase(it);
		}
	}

	void Host::try_update_expected_time_of_completion(milliseconds run_time) noexcept
	{
		const ms expected_completion_time = cluster->simulation->get_current_time() + run_time;
		if (expected_completion_time > expected_time_of_completion)
			expected_time_of_completion = expected_completion_time;
	}


	void finish_job_from_host(Host& host, Job& job)
	{
		
	}
}

template<> const std::vector<std::string> Utils::enum_strings<cs::HostStatus>::data = {
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