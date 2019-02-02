#pragma once
#include "SchedulerAlgorithm.h"

class Scheduler
{
public:
	explicit Scheduler(SchedulerAlgorithm& algorithm);

	void run(double currentTime, std::vector<Node>& cluster, Task& arrivedTask);

	const string& get_algorithm_name() const { return algorithm_.algorithm_name; }
	const bool is_algorithm_using_complete_signal();
private:
	SchedulerAlgorithm& algorithm_;
};

