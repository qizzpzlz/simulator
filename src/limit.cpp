#include "../includes/limit.h"
#include "../includes/job.h"
#include "../includes/queue.h"

namespace ClusterSimulator
{
    bool RuntimeLimit::is_eligible(const Queue& queue, const Host& host, const Job& job) const
    {
	    const auto search = entries_.find(&host);
        if (search == entries_.end()) return true;
        
        return job.run_time <= search->second;
    }

    bool HjobLimit::is_eligible(const Queue& queue, const Host& host, const Job& job) const
    {
	    const auto search = entries_.find(&host);
        if (search == entries_.end()) return true;

		Queue::HostInfo info;
		if (!queue.try_get_dispatched_host_info(host, &info)) return true;

		return info.slot_dispatched + job.slot_required <= search->second;
    }

	bool ExclusiveLimit::is_eligible(const Queue& queue, const Host& host, const Job& job) const
    {
		return host.num_current_jobs == 0;
    }
}