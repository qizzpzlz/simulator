#include "scenario.h"
#include "job.h"
#include "queue.h"
#include "cluster_simulation.h"

namespace ClusterSimulator
{
	std::size_t Job::id_gen_ = 0;
	const bool Job::USE_STATIC_HOST_TABLE = ClusterSimulation::USE_STATIC_HOST_TABLE_FOR_JOBS;

	using namespace std::chrono;

	Job::Job(const ScenarioEntry& entry, Queue& queue, const ms submit_time) :
		id{ id_gen_++ },
		slot_required{ entry.event_detail.num_slots },
		mem_required{ entry.event_detail.mem_req },
		submit_time{ submit_time },
		cpu_time{ double_to_milliseconds(entry.event_detail.job_cpu_time) },
		non_cpu_time{ double_to_milliseconds(entry.event_detail.job_non_cpu_time) },
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
	{
		if constexpr (ClusterSimulation::USE_STATIC_HOST_TABLE_FOR_JOBS)
		{
			eligible_hosts_.reserve(entry.eligible_indices.size());
			std::transform(entry.eligible_indices.begin(), entry.eligible_indices.end(), std::back_inserter(eligible_hosts_),
				[&cluster = queue.simulation_->get_cluster()](unsigned short index){return &cluster[index]; });
		}
	}

	//std::vector<Host*>& 

	std::vector<Host*> Job::get_eligible_hosts() const
	{
		if constexpr (ClusterSimulation::USE_STATIC_HOST_TABLE_FOR_JOBS)
		{
			std::vector<Host*> hosts;
			std::copy_if(eligible_hosts_.begin(), eligible_hosts_.end(), std::back_inserter(hosts),
				[slot_required = this->slot_required](Host* host_ptr){ return host_ptr->remaining_slots() >= slot_required; });
			return hosts;
		}
		
		return queue_managing_this_job->match(*this);
	}

	const std::vector<Host*>& Job::get_compatible_hosts() const
	{
		if constexpr (ClusterSimulation::USE_STATIC_HOST_TABLE_FOR_JOBS)
		{
			return eligible_hosts_;
		}
		else
		{
			throw std::runtime_error("Job::get_compatible_hosts() is not implemented for the case where USE_STATIC_HOST_TABLE_FOR_JOBS is false.");
		}
	}
}

