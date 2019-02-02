#pragma once
#include <string>
#include "Cluster.h"

class SchedulerAlgorithm
{
public:
	const std::string algorithm_name;
	const bool is_using_complete_signal;

	SchedulerAlgorithm(const std::string& name, bool isUsingCompleteSignal = false);
	virtual ~SchedulerAlgorithm() = default;
	virtual void run(vector<Node>& cluster, Task& arrivedTask) = 0;
};

