#include "event_item.h"
#include "cluster_simulation.h"

namespace cs {
	EventItem::EventItem(ScenarioEntry&& entry, ClusterSimulation& simulation)
	: time{ entry.timestamp }
	, priority{ 0 }
	, type{ Type::SCENARIO }
	{
		switch (entry.type)
		{
		case ScenarioEntry::Type::SUBMISSION:
			action = Submission{ std::move(entry), simulation };
			break;
		case ScenarioEntry::Type::CHANGE_STATUS:
			action = ChangeStatus{};
			break;
		default:
			throw std::runtime_error("Not implemented exception.");
		}
	}
	
	
	void Submission::operator()()
	{
		Queue* queue;
		if constexpr (config::USE_ONLY_DEFAULT_QUEUE)
		{
			queue = &simulation->get_default_queue();
		}
		else
		{
			if (entry.event_detail.queue_name == "-" || entry.event_detail.queue_name.empty())
				return;

			queue = &simulation->find_queue(entry.event_detail.queue_name);
		}

		queue->enqueue(std::make_shared<Job>(entry, *queue, simulation->get_current_time()));

		simulation->reserve_dispatch_event();

		simulation->increment_job_submission_counters();
		//log(LogLevel::info, "newly_submitted_jobs{0}", simulation.newly_submitted_jobs_);
	}

	
	void ChangeStatus::operator()()
	{
		//if (entry.event_detail.host_name.empty())
		//	return;

		//Host& host = simulation.get_cluster().find_node(entry.event_detail.host_name)->second;

		//host.set_status(entry.event_detail.host_status);
		//host.cpu_factor = entry.event_detail.cpu_factor;
		//host.ncpus = entry.event_detail.ncpus;
		//host.nprocs = entry.event_detail.nprocs;
		//host.ncores = entry.event_detail.ncores;
		//host.nthreads = entry.event_detail.nthreads;

		//Utils::enum_const_ref_holder<HostStatus> test = Utils::enum_to_string<HostStatus>(host.status);
		//std::stringstream ss;
		//ss << test;

		//log(LogLevel::info, "Host {0}'s status is changed to {1}", host.id, ss.str());
	}
} // namespace cs