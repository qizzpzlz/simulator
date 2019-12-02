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

	milliseconds Host::get_expected_run_time(const Job& job) const noexcept
	{
		// Estimated run time: cpu_time / factor + non_cpu_time
		return duration_cast<milliseconds>(job.cpu_time / cpu_factor + job.non_cpu_time);
	}

	void Host::execute_job(Job& job)
	{
		simulation->num_dispatched_slots += job.slot_required;
		if (job.total_pending_duration > 0ms)
			simulation->update_pending_duration(job.total_pending_duration);

		job.queue_managing_this_job->using_job_slots += job.slot_required;

		if constexpr (ClusterSimulation::LOG_ANY)
			simulation->log(LogLevel::info, "Job #{0} is executed in Host {1}"
				, job.id, this->get_name());

		const auto run_time{ duration_cast<milliseconds>(get_expected_run_time(job) * 0.75) };
		if (run_time < 0ms)
			throw std::runtime_error("run time can't be negative.");
		try_update_expected_time_of_completion(run_time);

		if constexpr (ClusterSimulation::LOG_ANY)
			if (slot_running_ + job.slot_required > max_slot)
				simulation->log(LogLevel::err, 
					"Host {0}: Slot required for job {1} cannot be fulfilled with this host.", id, job.id);

		slot_running_ += job.slot_required;
		num_current_running_slots += job.slot_required;
		num_current_jobs++;

		const ms start_time{ simulation->get_current_time() };
		job.start_time = start_time;
		job.finish_time = start_time + run_time;
		job.set_run_host_name(name_);
		job.state = JobState::RUN;

		// Reserve finish event
		simulation->after_delay(run_time, [this, job]() mutable -> void
			{
				this->exit_job(job);

				this->simulation->num_dispatched_slots -= job.slot_required;
				job.queue_managing_this_job->using_job_slots -= job.slot_required;

				this->simulation->log_using_slots();
				this->simulation->log_jobmart(job);
			});

		cluster->update();
	}

	
	void Host::exit_job(const Job& job)
	{
		slot_running_ -= job.slot_required;

		//Queue::HostInfo info;
		// if (!job.queue_managing_this_job->try_get_dispatched_host_info(*this, &info))
		// 	throw std::runtime_error("Queue managing a job does not have information about the host of the job.");

		//info.slot_dispatched -= job.slot_required;
		num_current_running_slots -=  job.slot_required;
		num_current_jobs--;

		// std::stringstream ss(job.get_exit_host_status());
		// ss >> Utils::enum_from_string<HostStatus>(this->status);

		if constexpr (ClusterSimulation::LOG_ANY)
			simulation->log(LogLevel::info,
				"Job #{0} is finished in Host {1}. (actual run time: {2} ms, scenario run time: {3} ms)"
				, job.id, this->get_name(), (job.finish_time - job.start_time).count()
				, job.cpu_time.count() + job.non_cpu_time.count());

		cluster->update();
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