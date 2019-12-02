#include "queue.h"
#include "scenario.h"
#include "queue_algorithm.h"

namespace ClusterSimulator
{
	void Scenario::add_scenario_entry(const ScenarioEntry& entry)
	{
		entries_.push(entry);
		unique_queues_.insert(entry.event_detail.queue_name);
		unique_apps_.insert(entry.event_detail.application_name);
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
		for (const auto& name : unique_queues_)
			queues.emplace_back(simulation, name);

		return queues;
	}

	//std::queue<Job> Scenario::generate_jobs() const
	//{
	//	const std::vector<ScenarioEntry>& entries = entries_;
	//	std::queue<Job> jobs;

	//	long origin = (*entries.begin()).timestamp;

	//	for (const auto& entry : entries)
	//	{
	//		if (entry.type == ScenarioEntry::ScenarioEntryType::CHANGE_STATUS)
	//			continue;

	//	}
	//}
}

