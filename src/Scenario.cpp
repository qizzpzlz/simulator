#include "../includes/queue.h"
#include "../includes/scenario.h"

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

	std::vector<Queue> Scenario::generate_queues(ClusterSimulation& simulation) const
	{
		//std::vector<Queue> queues(unique_queues_.size());
		//std::transform(unique_queues_.begin(), unique_queues_.end(), queues.begin(),
		//	[](const std::string& name) -> Queue
		//	{
		//		return Queue{ name };
		//	});

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

