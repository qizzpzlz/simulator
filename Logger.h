#pragma once
#ifndef LOGGER_H
#define LOGGER_H
#include "Task.h"
#include "Utils.h"
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <vector>
using namespace std;

class Logger
{
public:
	Logger();
	~Logger();
	void writeLog(Task& task);
	int createLogFile(const string& name);

	int clearLog();
private:
	vector<Task> logs_{};

};

#endif

