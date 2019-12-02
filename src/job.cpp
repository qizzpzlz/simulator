#include "scenario.h"
#include "job.h"
#include "queue.h"

namespace ClusterSimulator
{
	int Job::id_gen_ = 0;

	using namespace std::chrono;
	milliseconds dtom(double value)
	{
		return duration_cast<milliseconds>(duration<double>(value));
	}

	Job::Job(const ScenarioEntry& entry, Queue& queue, const ms submit_time) :
		id{ id_gen_++ },
		slot_required{ entry.event_detail.num_slots },
		mem_required{ entry.event_detail.mem_req },
		submit_time{ submit_time },
		cpu_time{ dtom(entry.event_detail.job_cpu_time) },
		non_cpu_time{ dtom(entry.event_detail.job_non_cpu_time) },
		queue_managing_this_job{ &queue }
		//run_time{ std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(entry.event_detail.job_run_time)) },
		//queue_managing_this_job{ std::make_shared<Queue>(queue) },
		//is_multi_host{entry.is_multi_host_submission},
		//swap_usage{entry.event_detail.job_swap_usage},
		//num_exec_procs{entry.event_detail.num_exec_procs},
		//application_name_{entry.event_detail.application_name},
		//dedicated_host_name_{ entry.event_detail.exec_hostname },
		//exit_host_status_{ entry.event_detail.job_exit_status },
		//mem_usage{ entry.event_detail.job_mem_usage },
	{ }

	std::vector<Host*> Job::get_eligible_hosts() const
	{
		return queue_managing_this_job->match(*this);
	}
}

