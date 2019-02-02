#include "TaskPool.h"


TaskPool::TaskPool() {
	nowTaskNum = 0;
	taskSrc = Random;
}

int TaskPool::flush(void) {
	nowTaskNum = 0;
	pool.clear();

	return 0;
}

int TaskPool::reloadTaskSeeker(void) {
	nowTaskNum = 0;

	return 0;
}


Task& TaskPool::dequeue_task() {
	/*
	buf.taskNumber = pool.at(nowTaskNum).taskNumber;
	buf.arrivalTime = pool.at(nowTaskNum).arrivalTime;
	buf.deadlineTime = pool.at(nowTaskNum).deadlineTime;
	strcpy_s(buf.name, pool.at(nowTaskNum).name);

	buf.core = 8;
	buf.estimateTime = 0;

	buf.startTime = 0;
	buf.executeTime = 0;
	buf.finishTime = 0;

	buf.leftTime = 0;

	buf.state = Unexecute;
	buf.energy = 0;

	nowTaskNum++;
	*/

	Task& task { pool.at(nowTaskNum) };
	nowTaskNum++;
	return task;
}

double TaskPool::getTime(void) {
	return pool.at(nowTaskNum).get_arrival_time();
}

bool TaskPool::isEmpty(void) {
	return pool.empty();
}

bool TaskPool::isEnd(void) {
	return (pool.size() == nowTaskNum);
}

double TaskPool::getSimulationStartTime(void) {
	return simulation_start_time_;
}

double TaskPool::getSimulationEndTime(void) {
	return simulation_end_time_;
}

double TaskPool::getTimeInterval(void) {
	double u, x;

	u = (double)rand() / (double)RAND_MAX;
	x = (-INTERVAL * log(u));

	return x;
}

double TaskPool::get_deadline(int benchmarkNumber) {
	static const double arrayDeadline[29] = { 86.41, 169.74, 3.49, 659.33, 478, 54.18, 46.75, 128.53, 492.13, 98.5, 450.67, 51.42, 353.43, 104.57, 12, 29.23, 5.39, 178.94, 439.28, 166.27, 6.68, 318.79, 850.46, 121.43, 189.37, 373.68, 588.67, 32, 214.41 };
	return arrayDeadline[benchmarkNumber];
}

int TaskPool::setTaskName(int benchmarkNumber, char* dest) const
{
	switch (benchmarkNumber) {
	case 0:
		strcpy_s(dest, 32, "perlbench");
		return 0;
	case 1:
		strcpy_s(dest, 32, "bzip2");
		return 0;
	case 2:
		strcpy_s(dest, 32, "gcc");
		return 0;
	case 3:
		strcpy_s(dest, 32, "bwaves");
		return 0;
	case 4:
		strcpy_s(dest, 32, "gamess");
		return 0;
	case 5:
		strcpy_s(dest, 32, "mcf");
		return 0;
	case 6:
		strcpy_s(dest, 32, "milc");
		return 0;
	case 7:
		strcpy_s(dest, 32, "zeusmp");
		return 0;
	case 8:
		strcpy_s(dest, 32, "gromacs");
		return 0;
	case 9:
		strcpy_s(dest, 32, "cactusADM");
		return 0;
	case 10:
		strcpy_s(dest, 32, "leslie3d");
		return 0;
	case 11:
		strcpy_s(dest, 32, "namd");
		return 0;
	case 12:
		strcpy_s(dest, 32, "gobmk");
		return 0;
	case 13:
		strcpy_s(dest, 32, "dealII");
		return 0;
	case 14:
		strcpy_s(dest, 32, "soplex");
		return 0;
	case 15:
		strcpy_s(dest, 32, "povray");
		return 0;
	case 16:
		strcpy_s(dest, 32, "calculix");
		return 0;
	case 17:
		strcpy_s(dest, 32, "hmmer");
		return 0;
	case 18:
		strcpy_s(dest, 32, "sjeng");
		return 0;
	case 19:
		strcpy_s(dest, 32, "GemsFDTD");
		return 0;
	case 20:
		strcpy_s(dest, 32, "libquantum");
		return 0;
	case 21:
		strcpy_s(dest, 32, "h264ref");
		return 0;
	case 22:
		strcpy_s(dest, 32, "tonto");
		return 0;
	case 23:
		strcpy_s(dest, 32, "lbm");
		return 0;
	case 24:
		strcpy_s(dest, 32, "omnetpp");
		return 0;
	case 25:
		strcpy_s(dest, 32, "astar");
		return 0;
	case 26:
		strcpy_s(dest, 32, "wrf");
		return 0;
	case 27:
		strcpy_s(dest, 32, "sphinx3");
		return 0;
	case 28:
		strcpy_s(dest, 32, "xalancbmk");
		return 0;
	}
	return 0;
}


int TaskPool::get_benchmark_number(char* name) {
	if (strcmp(name, "perlbench") == 0) {
		return 0;
	}
	else if (strcmp(name, "bzip2") == 0) {
		return 1;
	}
	else if (strcmp(name, "gcc") == 0) {
		return 2;
	}
	else if (strcmp(name, "bwaves") == 0) {
		return 3;
	}
	else if (strcmp(name, "gamess") == 0) {
		return 4;
	}
	else if (strcmp(name, "mcf") == 0) {
		return 5;
	}
	else if (strcmp(name, "milc") == 0) {
		return 6;
	}
	else if (strcmp(name, "zeusmp") == 0) {
		return 7;
	}
	else if (strcmp(name, "gromacs") == 0) {
		return 8;
	}
	else if (strcmp(name, "cactusADM") == 0) {
		return 9;
	}
	else if (strcmp(name, "leslie3d") == 0) {
		return 10;
	}
	else if (strcmp(name, "namd") == 0) {
		return 11;
	}
	else if (strcmp(name, "gobmk") == 0) {
		return 12;
	}
	else if (strcmp(name, "dealII") == 0) {
		return 13;
	}
	else if (strcmp(name, "soplex") == 0) {
		return 14;
	}
	else if (strcmp(name, "povray") == 0) {
		return 15;
	}
	else if (strcmp(name, "calculix") == 0) {
		return 16;
	}
	else if (strcmp(name, "hmmer") == 0) {
		return 17;
	}
	else if (strcmp(name, "sjeng") == 0) {
		return 18;
	}
	else if (strcmp(name, "GemsFDTD") == 0) {
		return 19;
	}
	else if (strcmp(name, "libquantum") == 0) {
		return 20;
	}
	else if (strcmp(name, "h264ref") == 0) {
		return 21;
	}
	else if (strcmp(name, "tonto") == 0) {
		return 22;
	}
	else if (strcmp(name, "lbm") == 0) {
		return 23;
	}
	else if (strcmp(name, "omnetpp") == 0) {
		return 24;
	}
	else if (strcmp(name, "astar") == 0) {
		return 25;
	}
	else if (strcmp(name, "wrf") == 0) {
		return 26;
	}
	else if (strcmp(name, "sphinx3") == 0) {
		return 27;
	}
	else if (strcmp(name, "xalancbmk") == 0) {
		return 28;
	}
	else {
		return 66;
	}
	return 66;
}

int TaskPool::setTaskSource(SOURCE src) {
	taskSrc = src;
	return 0;
}

SOURCE TaskPool::getTaskSource(void) {
	return taskSrc;
}

int TaskPool::setTaskFormFile(char* srcFileName) {
	strcpy_s(fileName, 256, srcFileName);
	if (taskSrc == Simulator) {
		return checkFileConsist();
	}
	else if (taskSrc == Original) {
		return checkFileConsist_Real();
	}
	else {
		return 1;
	}
}

char* TaskPool::getFileName() {
	return fileName;
}

void TaskPool::fill(double timeInterval)
{
	simulation_start_time_ = getCurrentTime();
	simulation_end_time_ = simulation_start_time_ + (double)timeInterval;

	double virtualNowTime = simulation_start_time_;
	double taskInterval = 0;

	//Task tempTask();
	int taskNumber = 0;
	int benchmarkSelect = 0;

	while (virtualNowTime < simulation_end_time_) 
	{
		benchmarkSelect = rand() % BENCHMARK_NUMBER;


		char temp[32];
		setTaskName(benchmarkSelect, temp);
		Task temptask = Task(taskNumber, virtualNowTime, std::string(temp));
		temptask.set_state(TaskState::Unexecuted);

		pool.push_back(temptask);

		taskNumber++;
		virtualNowTime += getTimeInterval();

		//delete[] temp;
	}
}

int TaskPool::checkFileConsist(void) {
	cout << endl;
	cout << "Check HEDUQUS log file consist..." << endl;
	if (_access(fileName, 04) == -1) {
		cout << "File not exist!" << endl;
		return 1;
	}

	FILE* fp;
	fopen_s(&fp, fileName, "rt");
	if (fp == NULL) {
		cout << "File open error!" << endl;
		return 2;
	}

	char buf[512];
	fgets(buf, 512, fp);

	int taskNum = 0;

	while (1) {
		int getNum = 0;
		char getName[32] = { 0, };
		double getAT = 0;
		double getDT = 0;

		fgets(buf, 512, fp);

		char* type;
		char* token = strtok_s(buf, "|", &type);
		int repeater = 0;
		while (token != NULL) {
			if (repeater == 0) {
				getNum = atoi(token);
			}
			else if (repeater == 1) {
				strcpy_s(getName, 32, token);
			}
			else if (repeater == 2) {
				getAT = atof(token);
			}
			else if (repeater == 3) {
				getDT = atof(token);
			}
			repeater++;
			token = strtok_s(NULL, "|", &type);
		}

		if ((taskNum != 0) && (getNum == 0)) {
			cout << "Hashed task number : " << pool.size() << endl;
			break;
		}
		else if (getNum == taskNum) {
			Task tempTask{ static_cast<unsigned long>(getNum), getAT, getName };//std::string("name") };
			pool.push_back(tempTask);
			taskNum++;
		}
	}

	fclose(fp);

	cout << "Checking Complete." << endl;

	return 0;
}

int TaskPool::checkFileConsist_Real(void) {
	cout << endl;
	cout << "Check original log file consist..." << endl;
	if (_access(fileName, 04) == -1) {
		cout << "File not exist!" << endl;
		return 1;
	}

	FILE* fp;
	fopen_s(&fp, fileName, "rt");
	if (fp == NULL) {
		cout << "File open error!" << endl;
		return 2;
	}

	char buf[512];
	int countNum = 0;
	int taskNum = 0;

	while (1) {
		int getNum = 0;
		char getName[32] = { 0, };
		double getAT = 0;
		double getDT = 0;

		fgets(buf, 512, fp);
		if (strlen(buf) < 40) {
			cout << "Hashed task number : " << pool.size() << endl;
			break;
		}

		char* type;
		char* token = strtok_s(buf, " ", &type);
		int repeater = 0;
		while (token != NULL) {
			if (repeater == 0) {
				getNum = atoi(token + 1);
			}
			else if (repeater == 2) {
				strcpy_s(getName, 32, token);
			}
			else if (repeater == 3) {
				getAT = atof(token);
			}
			else if (repeater == 4) {
				getDT = atof(token);
			}
			repeater++;
			token = strtok_s(NULL, " ", &type);
		}

		if (getNum >= countNum) {
			Task tempTask{ static_cast<unsigned long>(getNum), getAT, getName };
			
			pool.push_back(tempTask);
			taskNum++;
			if (getNum == countNum) {
				countNum++;
			}
			else {
				countNum = getNum + 1;
			}
		}
	}

	fclose(fp);

	cout << "Checking Complete." << endl;
	return 0;
}

int TaskPool::resettingTaskPool(int timeInterval) {
	simulation_start_time_ = getCurrentTime();
	simulation_end_time_ = simulation_start_time_ + (double)timeInterval;

	double diffSimulationTime = simulation_start_time_ - pool.at(0).get_arrival_time();

	for (unsigned int i = 0; i < pool.size(); i++) {
		pool.at(i).add_diff_to_arrival_time(diffSimulationTime);
		//pool.at(i).deadlineTime += diffSimulationTime;
		//printf("%f %f\n",pool.at(i).arrivalTime,pool.at(i).deadlineTime);
	}

	return pool.size();
}