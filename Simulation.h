#pragma once
#include <vector>
#include "Node.h"
#include "Logger.h"
#include <iostream>
#include "Scheduler.h"
#include "TaskPool.h"
#include "Task.h"

#define MAIN 0
#define SET_ENV 1
#define SET_TIME 1
#define SET_CORE 2
#define SET_ALGO 3


using namespace std;




//static Logger logger;

class Simulation
{
public:
	double simulationTime{7200 };
	int numNode;

	Simulation(SchedulerAlgorithm& algorithm, double simulationTime, int nodeNumber);
	~Simulation();

	void simulate();
	double get_time_elapsed();
	void reset();

	const Task& get_arrived_task() const;
	void set_arrived_task(Task task);
	void run_scheduler(double currentTime, Task& arrivedTask);
	
	void print_intro();
	void print_menu(int select);
	void print_result();

	void set_simulation_environment();

	//Task& operator=(const Task&) = default;

	vector<Node>& get_nodes() { return nodes_; }

private:
	std::vector<Node> nodes_;
	double time_elapsed_;
	Logger logger_;
	//Task arrived_task_;
	Scheduler scheduler_;
	Cluster cluster_;
	TaskPool taskPool_;

	
};

