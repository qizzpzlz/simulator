#include "../includes/scenario.h"
#include "../includes/job.h"
#include "../includes/queue.h"

namespace ClusterSimulator
{
	int Job::id_gen_ = 0;

	Job::Job(const ScenarioEntry& entry, const Queue& queue, const ms submit_time) :
		slot_required{entry.event_detail.num_slots},
		run_time{ std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(entry.event_detail.job_run_time)) },
		queue_managing_this_job{ std::make_shared<Queue>(queue) },
		is_multi_host{entry.is_multi_host_submission},
		mem_required{entry.event_detail.mem_req},
		//queue_managing_this_job{ std::make_shared<Queue>(queue) }
		swap_usage{entry.event_detail.job_swap_usage},
		num_exec_procs{entry.event_detail.num_exec_procs},
		submit_time{ submit_time },
		application_name_{entry.event_detail.application_name},
		dedicated_host_name_{ entry.event_detail.exec_hostname },
		exit_host_status_{ entry.event_detail.job_exit_status },
		mem_usage{ entry.event_detail.job_mem_usage },
		cpu_time{ entry.event_detail.job_cpu_time }
		
	{
		
	}
}

