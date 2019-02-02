#pragma once
#include "Task.h"
#include "States.h"
#include "VirtualQueue.h"
using namespace std;


class Node
{
public:
	const int Id{ id_gen_++ };

	//virtual ~Node();
	double execute(double current_time);
	//double kill();

	double get_current_time() const { return current_time_; }
	void set_current_time(double time);
	void set_task(Task&& task);
	Task& get_task() const;

	double get_left_time() const;
	double get_exe_time() const;
	NodeState get_state() const;

	void enqueue_task(Task& task);
	
	bool try_set_paused_task();
	bool try_set_task_from_queue();
	bool is_arrival_time_reached() const { return arrival_time_reached_; }
	void set_arrival_time_reached(const bool value) { arrival_time_reached_ = value; }


private:
	static int id_gen_;
	Task* current_task_ptr_ = nullptr;
	double current_time_ = 0;
	double left_time_ = 0;
	double exe_time_ = 0;
	NodeState state_{NodeState::Idle}; // Should be node state instead?
	Queue queue_;
	PausedQueue paused_queue_;
	bool arrival_time_reached_ = false;
};

