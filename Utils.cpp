#include "Utils.h"
#include <random>
#include <iterator>
#include <iostream>
//#include <consoleapi2.h>
//#include <processenv.h>
//#include <Windows.h>
//#include <sys/timeb.h>
using namespace std;

double getEstimateTime(Task& task) {
	/*
	static const double estimateTable[8][29] = {
		{43.24, 84.5, 1.62, 328.32, 238.11, 27.04, 22.72, 63.95, 245.74, 48.72, 224.46, 25.67, 176.12, 52.21, 5.81, 14.33, 2.53, 88.78, 218.47, 80.29, 3.34, 158.63, 423.02, 58.04, 94.07, 187.37, 293.52, 15.7, 107.22},
		{43.24, 84.5, 1.62, 328.32, 238.11, 27.04, 22.72, 63.95, 245.74, 48.72, 224.46, 25.67, 176.12, 52.21, 5.81, 14.33, 2.53, 88.78, 218.47, 80.29, 3.34, 158.63, 423.02, 58.04, 94.07, 187.37, 293.52, 15.7, 107.22},
		{40.21, 78.21, 1.51, 303.67, 219.55, 25.29, 21.78, 59.65, 227.17, 45.44, 208.18, 23.71, 163.04, 48.28, 5.41, 13.22, 2.36, 81.82, 202.47, 75.58, 3.09, 146.97, 392.1, 54.29, 87.19, 172.83, 271.57, 14.52, 98.44},
		{37.36, 72.86, 1.4, 282.19, 204.65, 23.82, 20.99, 56.13, 210.86, 43.19, 193.68, 22.03, 151.66, 44.91, 5.06, 12.25, 2.17, 76.03, 188.66, 71.72, 2.86, 136.44, 364.34, 51.09, 80.74, 161.44, 251.99, 13.56, 91.31},
		{34.81, 68.16, 1.31, 261.34, 190.38, 22.56, 20.36, 53.01, 196.32, 39.69, 180.95, 20.55, 141.77, 42, 4.78, 11.45, 2.03, 70.87, 176.49, 68.68, 2.68, 127.24, 340.3, 48.65, 74.86, 150.81, 233.17, 12.71, 85.78},
		{32.75, 64.06, 1.24, 246.17, 178.87, 21.47, 19.81, 50.21, 184.8, 37.54, 170.14, 19.32, 133.08, 39.38, 4.47, 10.73, 1.91, 66.42, 166.26, 65.78, 2.51, 119.32, 317.22, 46.27, 70.44, 141.86, 219.78, 11.93, 79.98},
		{30.91, 60.51, 1.17, 233.6, 167.99, 20.52, 19.35, 47.83, 174.08, 35.71, 160.55, 18.14, 125.43, 37.15, 4.23, 10.12, 1.79, 62.54, 156.89, 63.41, 2.36, 112.46, 301.62, 44.34, 67.01, 133.73, 208.26, 11.34, 75.67},
		{29.25, 57.37, 1.11, 220.39, 158.54, 19.66, 18.85, 45.5, 164.65, 34.03, 151.92, 17.16, 118.99, 35.16, 4.05, 9.64, 1.7, 59.05, 148.18, 60.95, 2.23, 106.27, 284.35, 42.65, 63.29, 126.67, 196.43, 10.7, 71.32}};
		*/

	static const double estimateTable[29] = 
	{43.24, 84.5, 1.62, 328.32, 238.11, 27.04, 22.72, 63.95, 245.74, 48.72, 224.46, 25.67, 176.12, 52.21, 5.81, 14.33, 2.53, 88.78, 218.47, 80.29, 3.34, 158.63, 423.02, 58.04, 94.07, 187.37, 293.52, 15.7, 107.22};

	return estimateTable[task.benchmark_num];
}
void printAssign(Task& task) 
{
	//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	
	cout.precision(2);
	cout << "[Task Assigned] Task#: #" << task.id << " | Name: " << task.name << " | Node: "
		<< task.get_executed_core_id() << " | Est: " << task.estimatedTime << std::endl;
}

void printArrive(Task& task)
{
	//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

	cout.precision(2);
	cout << "[Task Arrived ] Task#: #" << task.id << " | Name: " << task.name << " | Arrival Time: "
		<< task.get_arrival_time() << endl;
}

double getCurrentTime(void)
{
	// TODO:
	//struct _timeb timeBuf;

	//LARGE_INTEGER Now;
	//LARGE_INTEGER Frequency;

	//QueryPerformanceFrequency(&Frequency);
	//_ftime64_s(&timeBuf);
	//QueryPerformanceCounter(&Now);

	//double usec = ((double)Now.QuadPart / (double)Frequency.QuadPart);
	//usec -= floor(usec);

	//return (double)timeBuf.time + usec;


	return 0.0;
}
bool compareNum(Task p, Task s) {
	return p.id < s.id;
}

bool compareStart(Task p, Task s) {
	return p.get_start_time() < s.get_start_time();
}


bool compareEst(Task p, Task s) {
	return p.estimatedTime < s.estimatedTime;
}
int taskStatusItoa(char* dest, int status) {
	switch (status) {
	case 0:
		strcpy_s(dest, 32, "Drop");
		break;
	case 1:
		strcpy_s(dest, 32, "Fail");
		break;
	case 2:
		strcpy_s(dest, 32, "Complete");
		break;
	case 3:
		strcpy_s(dest, 32, "Pause");
		break;
	case 4:
		strcpy_s(dest, 32, "Migrate");
		break;
	case 5:
		strcpy_s(dest, 32, "Swap");
		break;
	default:
		strcpy_s(dest, 32, "Error");
		break;
	}

	return 0;
}

int taskNameAtoi(char* taskName) {
	if (!strncmp(taskName, "per", 3)) {
		return 0;
	}
	else if (!strncmp(taskName, "bzi", 3)) {
		return 1;
	}
	else if (!strncmp(taskName, "gcc", 3)) {
		return 2;
	}
	else if (!strncmp(taskName, "bwa", 3)) {
		return 3;
	}
	else if (!strncmp(taskName, "gam", 3)) {
		return 4;
	}
	else if (!strncmp(taskName, "mcf", 3)) {
		return 5;
	}
	else if (!strncmp(taskName, "mil", 3)) {
		return 6;
	}
	else if (!strncmp(taskName, "zeu", 3)) {
		return 7;
	}
	else if (!strncmp(taskName, "gro", 3)) {
		return 8;
	}
	else if (!strncmp(taskName, "cac", 3)) {
		return 9;
	}
	else if (!strncmp(taskName, "les", 3)) {
		return 10;
	}
	else if (!strncmp(taskName, "nam", 3)) {
		return 11;
	}
	else if (!strncmp(taskName, "gob", 3)) {
		return 12;
	}
	else if (!strncmp(taskName, "dea", 3)) {
		return 13;
	}
	else if (!strncmp(taskName, "sop", 3)) {
		return 14;
	}
	else if (!strncmp(taskName, "pov", 3)) {
		return 15;
	}
	else if (!strncmp(taskName, "cal", 3)) {
		return 16;
	}
	else if (!strncmp(taskName, "hmm", 3)) {
		return 17;
	}
	else if (!strncmp(taskName, "sje", 3)) {
		return 18;
	}
	else if (!strncmp(taskName, "Gem", 3)) {
		return 19;
	}
	else if (!strncmp(taskName, "lib", 3)) {
		return 20;
	}
	else if (!strncmp(taskName, "h26", 3)) {
		return 21;
	}
	else if (!strncmp(taskName, "ton", 3)) {
		return 22;
	}
	else if (!strncmp(taskName, "lbm", 3)) {
		return 23;
	}
	else if (!strncmp(taskName, "omn", 3)) {
		return 24;
	}
	else if (!strncmp(taskName, "ast", 3)) {
		return 25;
	}
	else if (!strncmp(taskName, "wrf", 3)) {
		return 26;
	}
	else if (!strncmp(taskName, "sph", 3)) {
		return 27;
	}
	else if (!strncmp(taskName, "xal", 3)) {
		return 28;
	}
	else {
		return 66;
	}
}
