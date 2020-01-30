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
	 * Estimate the expected completion time of a specified job on this host.
	 * Expected completition time is the sum of expected execution time of the job
	 * and the availability time of this host after completing previously assigned jobs.
	 */
	milliseconds Host::get_expected_completion_duration(const Job& job) const noexcept
	{
		if (!is_compatible(job)) return milliseconds::max();	// Returns infinity if it can't be run.

		const int slot_required = job.slot_required;
		int remaining = remaining_slots();
		if (slot_required <= remaining) return get_expected_run_time(job);

		// Running jobs are sorted with it's finish time.
		// Therefore we need to iterate the list in reversed order
		// so that we can get expected available slots at future times in the correct order.
		for (auto it = running_jobs_.rbegin(); it != running_jobs_.rend(); ++it)
		{
			auto& running_job = **it;
			remaining += running_job.slot_required;
			if (slot_required <= remaining)
			{
				const milliseconds ready_duration = running_job.finish_time - simulation->get_current_time();
				return get_expected_run_time(job) + ready_duration;
			}
		}

		throw std::runtime_error("Can't get expected completion duration.");
	}

	milliseconds Host::get_expected_run_time(const Job& job) const noexcept
	{
		// Estimated run time: cpu_time / factor + non_cpu_time
		return duration_cast<milliseconds>(job.cpu_time / cpu_factor + job.non_cpu_time);
	}

	/**
	 * Execute a specified job in this host.
	 * This will cause job finished event after the estimated runtime for
	 * running the job in this host.
	 * The runtime is calculated by: job.cpu_time / host.cpu_factor + job.non_cpu_time .
	 * **The ownership of the job is transferred to this host.
	 */
	void Host::execute_job(std::unique_ptr<Job> job_ptr)
	{
		Job& job{ *job_ptr };

		// Update dispatched slots
		simulation->num_dispatched_slots += job.slot_required;
		if (job.total_pending_duration > 0ms)
			simulation->update_pending_duration(job.total_pending_duration);

		job.queue_managing_this_job->using_job_slots += job.slot_required;

		if constexpr (ClusterSimulation::LOG_ANY)
			simulation->log(LogLevel::info, "Job #{0} is executed in Host {1}"
				, job.id, this->get_name());

		// Estimate run time for the job in this host.
		const auto run_time{ duration_cast<milliseconds>(get_expected_run_time(job) * ClusterSimulation::RUNTIME_MULTIPLIER) };
		if (run_time < 0ms)
			throw std::runtime_error("run time can't be negative.");

		try_update_expected_time_of_completion(run_time);

		if constexpr (ClusterSimulation::LOG_ANY)
			if (num_current_running_slots + job.slot_required > max_slot)
				simulation->log(LogLevel::err, 
					"Host {0}: Slot required for job {1} cannot be fulfilled with this host.", id, job.id);

		const ms start_time{ simulation->get_current_time() };
		job.start_time = start_time;
		job.finish_time = start_time + run_time;
		job.set_run_host_name(name_);
		job.state = JobState::RUN;

		// Calculate Queuing Time here.
		auto q_time = start_time - job.submit_time;
		simulation->update_total_queuing_time(q_time);

		// Reserve job finished event.
		simulation->after_delay(run_time, std::bind(&Host::exit_job, this));

		cluster->update();

		// Register job to this host
		num_current_running_slots += job.slot_required;
		running_jobs_.push_back(std::move(job_ptr));
		// Sort running_jobs_ here; ordering depends on finish time of job
		std::sort(running_jobs_.begin(), running_jobs_.end(),
			[](std::unique_ptr<Job>& a, std::unique_ptr<Job>& b)
			{
				return a->finish_time > b->finish_time;
			});
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
				"Job #{0} is finished in Host {1}. (actual run time: {2} ms, scenario run time: {3} ms)"
				, job.id, this->get_name(), (job.finish_time - job.start_time).count()
				, job.cpu_time.count() + job.non_cpu_time.count());

		cluster->update();

		simulation->num_dispatched_slots -= job.slot_required;
		job.queue_managing_this_job->using_job_slots -= job.slot_required;

		simulation->log_using_slots();
		simulation->log_jobmart(job);

		running_jobs_.pop_back();
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