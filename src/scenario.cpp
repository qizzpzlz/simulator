#include "queue.h"
#include "scenario.h"
#include "queue_algorithm.h"

namespace cs
{
	void Scenario::add_scenario_entry(ScenarioEntry&& entry)
	{
		unique_queues_.insert(entry.event_detail.queue_name);
		unique_apps_.insert(entry.event_detail.application_name);
		entries_.push(std::move(entry));
	}

	const ScenarioEntry Scenario::pop()
	{
		ScenarioEntry entry = std::move(entries_.front());
		entries_.pop();
		return entry; // Move
	}

	std::vector<ScenarioEntry> Scenario::pop_all_latest()
	{
		std::vector<ScenarioEntry> result;
		result.push_back(std::move(entries_.front()));
		ms time = result.back().timestamp;

		entries_.pop();
		if (entries_.empty()) return result;

		ms next_time = entries_.front().timestamp;
		while (next_time == time)
		{
			result.push_back(std::move(entries_.front()));
			entries_.pop();
			if (entries_.empty()) break;
			next_time = entries_.front().timestamp;
		}
		return result;
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

