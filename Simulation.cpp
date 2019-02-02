#include "Simulation.h"


Simulation::Simulation(SchedulerAlgorithm& algorithm, double simulationTime, int nodeNumber)
	: scheduler_{algorithm}, simulationTime{simulationTime}
{
	for (int i = 0; i < nodeNumber; i++)
		nodes_.emplace_back();
}


Simulation::~Simulation() = default;

void Simulation::simulate() 
{

	if (simulationTime == 0) 
	{
		cout << "Simulation End. Reason : Simulation Time Interval is 0" << endl;
	}

	logger.clearLog();
	//vQ.flush();
	//vPQ.flush();


	//	cout<< "1" << endl;
	//if (taskPool_.getTaskSource() == Random) {
		taskPool_.flush();
		taskPool_.fill(simulationTime);
	//}
	//else {
	//	taskPool_.reloadTaskSeeker();
	//	taskPool_.resettingtaskPool_(simulationTime);
	//}

	//	cout<< "2 " << endl;


	for (auto& node : nodes_)
		node.set_current_time(taskPool_.getSimulationStartTime());


	//	cout << "3" << endl;

		//최초 한 번 도착한 태스크에 대해 스케줄러 실행
	double nextAT = taskPool_.getTime(); //nextArrivalTime

//	cout<< "4" << endl;

	Task* arrived_task = &taskPool_.dequeue_task();


	//	cout<< "5" << endl;
	printArrive(*arrived_task);
	//	cout<< "6" << endl;
	run_scheduler(nextAT, *arrived_task);
	//	cout<< "7" << endl;


		//이제 시뮬레이터 종료까지 계속 스케줄러와 CPU를 실행한다.
	while (!taskPool_.isEnd()) {
		nextAT = taskPool_.getTime();

		bool allCoreLeachAT = false;

		while (!allCoreLeachAT) {

			//코어에 작업을 로드
			for (auto& node : nodes_)
			{
				while (node.get_state() == NodeState::Idle)
				{
					if (!node.try_set_paused_task() && !node.try_set_task_from_queue())
					{
						//할 작업이 없음 -> 다음 작업 도착까지 대기
						node.set_arrival_time_reached(true);
						break;
					}

				}
			}		

			//double completeTime[8] = { nextAT,nextAT,nextAT,nextAT,nextAT,nextAT,nextAT,nextAT };

			//실행
			for (auto& node : nodes_)
			{

				if (node.is_arrival_time_reached()) {
					//할 작업이 없음
				}
				else if (node.get_current_time() == nextAT) {
					//이미 다음 작업이 도착 시간까지 실행완료
					node.set_arrival_time_reached(true);
				}
				else if (nextAT < node.get_current_time() + node.get_left_time()) {
					//실행 완료시간이 다음 작업 도착시간을 넘기면 도착시간까지만 실행
					node.execute(nextAT);
					node.set_arrival_time_reached(true);
				}
				else {
					//if (completeSignalOn) {
					//	//만약에 완료 신호가 켜져있으면 임시로 실행시킨다.
					//	completeTime[i] = vCore.getCoreTime(i) + vCore.getLeftTime(i);
					//}
					/*else {*/
						//완료신호가 없을 때는 통상적으로 실행시킨다.
						node.execute(nextAT);
					/*}*/
				}
			}
			

			//if (completeSignalOn) {
			//	double smallestTime = completeTime[7];

			//	for (int i = 0; i < 8; i++) {
			//		if (completeTime[i] < smallestTime) {
			//			smallestTime = completeTime[i];
			//		}
			//	}

			//	//제일 빠른 시간까지 모든 코어를 실행
			//	for (int i = 0; i < 8; i++) {
			//		vCore.execute(i, smallestTime);
			//	}

			//	//완료 스케쥴러를 실행한다.
			//	if (nextAT != smallestTime) {
			//		cScheduler(smallestTime);
			//	}
			//}

			//모든 코어가 다음 작업 도착시간에 왔는지 체크.
			allCoreLeachAT = true;
			for (auto& node : nodes_) {
				allCoreLeachAT &= node.is_arrival_time_reached();
			}
		}

		//모든 코어가 다음 작업이 도착한 시간까지 왔으므로 다음 작업에 대해 스케줄러 실행.

		arrived_task = &taskPool_.dequeue_task();
		printArrive(*arrived_task);
		run_scheduler(nextAT, *arrived_task);

		// Reset arrival indicators
		for (auto& node : nodes_)
		{
			node.set_arrival_time_reached = false;
		}
	}
	for (auto& node : nodes_) {
		nodes_= 
	}

	//여기에 시뮬레이션 종료까지 작동하는 코드를 집어넣어야합니다.
	//현재는 Task Pool이 모두 비어버리면 바로 종료되는 형식이므로
	//Task 도착 - 시뮬레이션 종료사이의 미묘한 시간이 실행되지 않고 있습니다.

	//vCore.failAllTask(taskPool.getSimulationEndTime());

	//for (int i = 0; i < 8; i++) {
	//	unsigned int size = vPQ.size(i);
	//	Task temp;

	//	for (unsigned int j = 0; j < size; j++) {

	//		vPQ.deQueue(i, temp);

	//		temp.finishTime = taskPool.getSimulationEndTime();
	//		temp.startTime = taskPool.getSimulationEndTime();
	//		temp.state = Fail;

	//		logger.writeLog(i, &temp);
	//	}
	//}

	//for (int i = 0; i < 8; i++) {
	//	unsigned int size = vQ.size(i);
	//	for (unsigned int j = 0; j < size; j++) {
	//		Task temp;

	//		vQ.deQueue(i, temp);

	//		temp.core = 8;
	//		temp.finishTime = 0;
	//		temp.startTime = 0;
	//		temp.state = Drop;

	//		logger.writeLog(8, &temp);
	//	}
	//}



	logger.createLogFile(scheduler_.get_algorithm_name());
}

//const Task& Simulation::get_arrived_task() const
//{
//	return arrived_task_;
//}

//void Simulation::set_arrived_task(Task task)
//{
//	arrived_task_ = task;
//}

void Simulation::run_scheduler(double currentTime, Task& arrivedTask)
{
	scheduler_.run(currentTime, nodes_, arrivedTask);
}

