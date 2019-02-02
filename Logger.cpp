#pragma once
#include "Logger.h"
#include <vector>
#include <fstream>
#include <chrono>
#include <ctime>


Logger::Logger()
{
}


Logger::~Logger()
{
}

void Logger:: writeLog(Task& task) 
{
	//TODO:
	//setEnergy(task);
	logs_.push_back(task);
}

int Logger:: createLogFile(const string& name) {
	//TODO:
	ofstream out_log("logs.txt");
	

	//sort(logs_.begin(), logs_.end(), compareStart);
	//stable_sort(logs_.begin(), logs_.end(), compareNum);

	//double numAllTask = ((double)logs_.back().id) + 1;
	double numAllTask = 0;
	double numComplete = 0;
	double numDrop = 0;
	double numFail = 0;

	//for (unsigned int i = 0; i < logs_.size(); i++) 
	//{
	//	numComplete++;
	//}


	//out_log << "task#|task ID|arrival time|deadline|core|start time|finish time|exe time|status|energy" << endl;

	for (unsigned int i = 0; i < logs_.size(); i++) {
		out_log << logs_.at(i).print();
	}

	auto currentTime = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(currentTime);
	out_log << "Simulation Start Time : ";
	out_log<<std::ctime(&time)<<endl;
	out_log<<"total task number : "<< numAllTask<<endl;
	out_log.close();
	return 0;
}
int Logger::clearLog()
{
	logs_.clear();
	return 0;
}
