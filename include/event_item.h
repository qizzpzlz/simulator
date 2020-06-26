#pragma once
#include "scenario.h"
#include <functional>

namespace cs {
using Action = std::function<void()>;

struct EventItem
{
	enum class Type { SCENARIO, JOB_FINISHED, JOB_RESERVED, DISPATCH, LOG };
	inline static const std::string type_strings[] = { "Scenario", "Job Finished", "Job Reserved", "Dispatch", "Log" };

	// TODO: make atomic
	std::size_t id = id_gen_++;
	ms time;
	Action action;
	uint8_t priority;
	Type type;

	EventItem(ms time, const Action& action, uint8_t priority = 0, Type type = Type::SCENARIO)
		: time{ time }
		, action{ action }
		, priority{ priority }
		, type{ type } {}

	/**
	 * Create an event item from a scenario entry.
	 * The entry is consumed here.
	 */
	EventItem(ScenarioEntry&& entry, ClusterSimulation& simulation);

	[[nodiscard]] const std::string& get_type_string() const noexcept
	{
		return type_strings[static_cast<int>(type)];
	}

	bool operator<(const EventItem& a) const
	{
		return a.time == time
			? a.priority < priority
			: a.time < time;
	}

private:
	inline static std::size_t id_gen_ = 0;
};

/**
 * Functor for the action of submission.
 */
struct Submission
{
	ScenarioEntry entry;
	ClusterSimulation* simulation;
	
	Submission(ScenarioEntry&& entry, ClusterSimulation& simulation)
		: entry{std::move(entry)}, simulation{&simulation} {}
	
	void operator()();
};

struct ChangeStatus
{
	void operator()();
};

} // namespace ClusterSimulator