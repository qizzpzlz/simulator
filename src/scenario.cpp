#include "../includes/queue.h"
#include "../includes/scenario.h"
#include "queue_algorithm.h"

namespace ClusterSimulator
{
	void Scenario::add_scenario_entry(const ScenarioEntry& entry)
	{
		entries_.push(entry);
		unique_queues_.insert(entry.event_detail.queue_name);
		unique_apps_.insert(entry.event_detail.application_name);

		if (entry.type == ScenarioEntry::ScenarioEntryType::SUBMISSION && 
			entry.event_detail.mem_req > max_mem_required)
			max_mem_required = entry.event_detail.mem_req;
	}

	const ScenarioEntry Scenario::pop()
	{
		ScenarioEntry entry = entries_.front(); // Copy
		entries_.pop();
		return entry; // Move
	}

	std::pair<std::vector<ScenarioEntry>, ms> Scenario::pop_all_latest()
	{
		ScenarioEntry front = std::move(entries_.front());
		std::vector<ScenarioEntry> result{ front };
		ms time = front.timestamp;

		entries_.pop();
		if (entries_.empty()) return std::make_pair(result, time);

		ms next_time = entries_.front().timestamp;
		while (next_time == time)
		{
			result.push_back(entries_.front());
			entries_.pop();
			if (entries_.empty()) break;
			next_time = entries_.front().timestamp;
		}
		return std::make_pair(result, time);
	}

	std::vector<Queue> Scenario::generate_queues(ClusterSimulation& simulation) const
	{
		std::vector<Queue> queues;
		queues.emplace_back(simulation, "default");

		return queues;
	}
}

