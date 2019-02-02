#include "Scheduler.h"

Scheduler::Scheduler(SchedulerAlgorithm& algorithm) : algorithm_{algorithm}
{

}

void Scheduler::run(double currentTime, std::vector<Node>& cluster, Task& arrivedTask)
{
	algorithm_.run(cluster, arrivedTask);
}
