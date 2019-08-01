#include "../includes/Limit.h"
#include "../includes/Job.h"
#include "../includes/Queue.h"

namespace ClusterSimulator
{
    bool RuntimeLimit::is_eligible(const Queue& queue, const Host& host, const Job& job) const
    {
        auto search = entries_.find(&host);
        if (search == entries_.end()) return true;
        
        return job.run_time <= search->second;
    }

    bool HjobLimit::is_eligible(const Queue& queue,const Host& host, const Job& job) const
    {
        auto search = entries_.find(&host);
        if (search == entries_.end()) return true;
        auto search_hosts = queue.dispatched_hosts_.find(&host);
        if (search_hosts != queue.dispatched_hosts_.end())
        {
            return search_hosts->second.slot_dispatched + job.slot_required 
                <= search->second;
        }

        return true;
    }
}