#include "limit.h"
#include "job.h"
#include "queue.h"
#include <stdexcept>

namespace ClusterSimulator
{
    bool RuntimeLimit::is_eligible(const Queue& queue, const Host& host, const Job& job) const
    {
	    const auto search = entries_.find(&host);
        if (search == entries_.end()) return true;
        
        return host.get_expected_run_time(job) <= search->second;
    }

    bool HjobLimit::is_eligible(const Queue& queue, const Host& host, const Job& job) const
    {
	    const auto search = entries_.find(&host);
        if (search == entries_.end()) return true;

      Queue::HostInfo info;
      // if (!queue.try_get_dispatched_host_info(host, &info)) return true;
      throw std::runtime_error("");

      return info.slot_dispatched + job.slot_required <= search->second;
    }

	bool ExclusiveLimit::is_eligible(const Queue& queue, const Host& host, const Job& job) const
    {
		return host.num_running_jobs() == 0;
    }
}
