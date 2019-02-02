#pragma once
#include "SchedulerAlgorithm.h"
#include "Utils.h"

class RandomAlgorithm : public SchedulerAlgorithm
{
public:
	RandomAlgorithm() : SchedulerAlgorithm("Random"){}

	void run(vector<Node>& cluster, Task& arrivedTask) override
	{
		//vector<Node> nodes = cluster.get_all_nodes();
		(*select_randomly(cluster.begin(), cluster.end())).enqueue_task(arrivedTask);
	}
};

