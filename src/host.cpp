#include "../includes/job.h"
#include "../includes/host.h"
#include "../dependencies/spdlog/spdlog.h"
#include "../includes/cluster_simulation.h"

namespace ClusterSimulator
{
	int Host::id_gen_ = 0;
	std::random_device Host::rd_{};
	std::mt19937 Host::gen_(rd_());
	std::uniform_int_distribution<> Host::dist_(1, MAX_RAND_NUM);

	std::chrono::milliseconds Host::get_expected_run_time(const Job& job) const noexcept
	{
		// int original_factor = cluster->simulation->find_host(job.get_dedicated_host_name()).cpu_factor;
		// double ratio = original_factor / static_cast<double>(cpu_factor);
		// return std::chrono::duration_cast<std::chrono::milliseconds>(job.run_time * ratio);
		return std::chrono::duration_cast<std::chrono::milliseconds>(job.run_time / cpu_factor);
	}

	void Host::execute_job(Job& job)
	{
		simulation->num_dispatched_slots += job.slot_required;
		job.queue_managing_this_job->using_job_slots += job.slot_required;
		ClusterSimulation::log(LogLevel::info, "Job #{0} is executed in Host {1}"
				,job.id, this->get_name());
		const auto run_time{ get_expected_run_time(job) };
		try_update_expected_time_of_completion(run_time);
		
		if (slot_running_ + job.slot_required > max_slot)
			ClusterSimulation::log(LogLevel::err, 
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
		simulation->after_delay(run_time,  [this, job] () mutable -> void
		{
			//ClusterSimulation::log(LogLevel::info, "oo cnt : {0}, queue name: {1}" , using_job_slots(), name);
			this->exit_job(job);
			std::stringstream ss(job.get_exit_host_status());
			ss >> Utils::enum_from_string<HostStatus>(this->status);
			ClusterSimulation::log(LogLevel::info, 
				"Job #{0} is finished in Host {1}. (actual run time: {2} ms, scenario run time: {3} ms)"
				,job.id, this->get_name(), (job.finish_time - job.start_time).count(), job.run_time.count());
			
			// dispatched_hosts_[best_host].slot_dispatched -= job.slot_required;
			this->simulation->num_dispatched_slots -= job.slot_required;
			job.queue_managing_this_job->using_job_slots -= job.slot_required;
		
			this->simulation->log_using_slots();
			ClusterSimulation::log_jobmart(job);
		});
	}

	void Host::exit_job(const Job& job)
	{
		slot_running_ -= job.slot_required;

		//Queue::HostInfo info;
		// if (!job.queue_managing_this_job->try_get_dispatched_host_info(*this, &info))
		// 	throw std::runtime_error("Queue managing a job does not have information about the host of the job.");

		//info.slot_dispatched -= job.slot_required;
		num_current_running_slots -=  job.slot_required;
		num_current_jobs --;
	}

	void Host::try_update_expected_time_of_completion(std::chrono::milliseconds run_time) noexcept
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