#include "../includes/Scenario.h"
#include "../includes/Job.h"

namespace ClusterSimulator
{
	int Job::id_gen_ = 0;

	Job::Job(const ScenarioEntry& entry, const Queue& queue) :
		application_name{entry.event_detail.application_name},
		slot_required{entry.event_detail.num_slots},
		run_time{ std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(entry.event_detail.job_run_time)) },
		dedicated_host_name_{ entry.event_detail.exec_hostname },
		exit_host_status_{ entry.event_detail.job_exit_status }
		//queue_managing_this_job{ std::make_shared<Queue>(queue) }
	{
		
	}
}

