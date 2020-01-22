#pragma once
#include <vector>
#include "job.h"
#include <set>
#include <algorithm>

namespace genetic
{
	struct HostInfo
	{
		float cpu_factor;
		unsigned char max_slots;

		bool write(std::streambuf& buf) const
		{
			const auto size = sizeof(HostInfo);
			return buf.sputn(reinterpret_cast<const char*>(this), size);
		}
	};
	
	struct Host
	{
		//HostInfo* info;
		unsigned char slots_remaining;
		std::vector<Job> allocated_jobs;
		//std::set<Job>
		
		//[[nodiscard]] unsigned char slots_remaining() const { return max_slots - slots_used; }

		void allocate_immediately(Job&& job, float cpu_factor)
		{
			auto estimated_time = job.cpu_time() / cpu_factor + job.non_cpu_time();
			
			job.set_finish_time(estimated_time);
			
			push_and_sort(job);
		}

		void allocate(Job&& job, uint32_t delay, float cpu_factor)
		{
			auto estimated_time = job.cpu_time() / cpu_factor + job.non_cpu_time();

			job.set_finish_time(estimated_time, delay);

			push_and_sort(job);
		}

		void push_and_sort(Job& job)
		{
			allocated_jobs.push_back(job);
			std::sort(allocated_jobs.begin(), allocated_jobs.end(),
				[](Job& a, Job& b)
				{
					return a.finish_time() > b.finish_time();
				});
			slots_remaining -= job.slots();
		}
	};
}
