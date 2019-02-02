#pragma once
#ifndef TASKPOOL_H
#define TASKPOOL_H

#include <iostream>
#include <stdio.h>

#include <vector>

#include <sys/types.h>
#include <sys/timeb.h>
#include <Windows.h>
#include <math.h>
#include <io.h>

#include "Task.h"
#include "Utils.h"

using namespace std;

typedef enum { Random, Simulator, Original } SOURCE;

class TaskPool {
public:
	TaskPool();

	int flush(void);
	int reloadTaskSeeker(void);

	int resettingTaskPool(int timeInterval);

	Task& dequeue_task();
	double getTime(void);
	bool isEmpty(void);
	bool isEnd(void);

	double getSimulationStartTime(void);
	double getSimulationEndTime(void);

	int setTaskSource(SOURCE src);
	SOURCE getTaskSource(void);

	int setTaskFormFile(char* srcFileName);

	char* getFileName(void);

	void fill(double timeInterval);

private:
	const static int BENCHMARK_NUMBER = 29;
	const static int INTERVAL = 6;

	int nowTaskNum;

	vector<Task> pool{};

	double simulation_start_time_;
	double simulation_end_time_;

	static double get_deadline(int benchmarkNumber);
	int setTaskName(int benchmarkNumber, char* dest) const;
	double getTimeInterval(void);
	static int get_benchmark_number(char* name);

	int checkFileConsist(void);
	int checkFileConsist_Real(void);

	SOURCE taskSrc;

	char fileName[256];

};

#endif


const double ETC[29][8] =	// ETC matrix
{ {43.24,43.24,43.24,43.24,29.25,29.25,29.25,29.25},
{84.50,84.50,84.50,84.50,57.37,57.37,57.37,57.37},
{1.62,1.62,1.62,1.62,1.11,1.11,1.11,1.11},
{328.32,328.32,328.32,328.32,220.39,220.39,220.39,220.39},
{238.11,238.11,238.11,238.11,158.54,158.54,158.54,158.54},
{27.04,27.04,27.04,27.04,19.66,19.66,19.66,19.66},
{22.72,22.72,22.72,22.72,18.85,18.85,18.85,18.85},
{63.95,63.95,63.95,63.95,45.50,45.50,45.50,45.50},
{245.74,245.74,245.74,245.74,164.65,164.65,164.65,164.65},
{48.72,48.72,48.72,48.72,34.03,34.03,34.03,34.03},
{224.46,224.46,224.46,224.46,151.92,151.92,151.92,151.92},
{25.67,25.67,25.67,25.67,17.16,17.16,17.16,17.16},
{176.12,176.12,176.12,176.12,118.99,118.99,118.99,118.99},
{52.21,52.21,52.51,52.51,35.16,35.16,35.16,35.16},
{5.81,5.81,5.81,5.81,4.05,4.05,4.05,4.05},
{14.33,14.33,14.33,14.33,9.64,9.64,9.64,9.64},
{2.53,2.53,2.53,2.53,1.70,1.70,1.70,1.70},
{88.78,88.78,88.78,88.78,59.05,59.05,59.05,59.05},
{218.47,218.47,218.47,218.47,148.18,148.18,148.18,148.18},
{80.29,80.29,80.29,80.29,60.95,60.95,60.95,60.95},
{3.34,3.34,3.34,3.34,2.23,2.23,2.23,2.23},
{158.63,158.63,158.63,158.63,106.27,106.27,106.27,106.27},
{423.02,423.02,423.02,423.02,284.35,284.35,284.35,284.35},
{58.04,58.04,58.04,58.04,42.65,42.65,42.65,42.65},
{94.07,94.07,94.07,94.07,63.29,63.29,63.29,63.29},
{187.37,187.37,187.37,187.37,126.67,126.67,126.67,126.67},
{293.52,293.52,293.52,293.52,196.43,196.43,196.43,196.43},
{15.70,15.70,15.70,15.70,10.70,10.70,10.70,10.70},
{107.22,107.22,107.22,107.22,71.32,71.32,71.32,71.32} };