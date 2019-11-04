#pragma once
#include <utility>
#include <functional>
#include <algorithm>
#include "queue.h"
#include "host.h"

namespace ClusterSimulator
{
	// TODO: As Template
	class QueueAlgorithm
	{
	public:
		virtual const std::string& get_name() const noexcept = 0;
		
		virtual void run(std::vector<Job>& jobs) const = 0;

		
	};

	/**
	 * Implementation of OLB Algorithm
	 */
	class OLBAlgorithm : public QueueAlgorithm
	{
		inline static const std::string name{"OLB"};
	public:
		const std::string& get_name() const noexcept override { return name; }

		void run(std::vector<Job>& jobs) const override
		{
			for (auto& job : jobs)
			{
				auto hosts = job.get_eligible_hosts();
				std::min_element(hosts.begin(), hosts.end(), 
					[](const Host* a, const Host* b)
					{
						return a->remaining_slots() < b->remaining_slots();
					});
			}
		}
	};

	class QueueAlgorithms
	{
	public:
		inline static const QueueAlgorithm* const OLB = new OLBAlgorithm();
	};
}



