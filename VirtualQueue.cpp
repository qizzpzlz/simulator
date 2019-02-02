#pragma once
#include "VirtualQueue.h"
#include "Logger.h"
#include "TaskPool.h"


VirtualQueue::~VirtualQueue() = default;

void VirtualQueue::enqueue(Task& task) 
{
	task.estimatedTime = getEstimateTime(task);
	task.leftTime = task.estimatedTime;

	list_.push_back(task);
	printAssign(task);
}

Task VirtualQueue::dequeue()
{
	Task task = list_.front();
	return task;
}

double VirtualQueue::get_time_left() {
	double sumLeft = 0;

	for (unsigned int i = 0; i < list_.size(); i++) {
		sumLeft += list_.at(i).leftTime;
	}

	return sumLeft;
}

