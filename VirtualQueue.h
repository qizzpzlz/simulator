#pragma once
#include "Task.h"

#include <vector>


class VirtualQueue
{
public:
	virtual ~VirtualQueue();
	virtual void enqueue(Task& task);
	virtual Task dequeue();
	
	double get_time_left();
	
	void clear() { list_.clear(); };
	std::size_t size() const { return list_.size(); }
	bool is_empty() const { return list_.empty(); }
protected:
	std::vector<Task> list_{};
};

class PausedQueue : public VirtualQueue
{
	
};

class Queue : public VirtualQueue
{
	
};

