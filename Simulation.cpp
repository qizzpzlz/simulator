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
	int input = 0;

	print_intro();

	while (1)
	{
	
		print_menu(MAIN);
		cin >> input;
		
		if (input == 1)
		{
			cout << "setting environment" << endl;
			set_simulation_environment();
		}
		else if (input == 2)
		{
			cout << "running simulation" << endl;

			if (simulationTime == 0)
			{
				cout << "Simulation End. Reason : Simulation Time Interval is 0" << endl;
			}

			logger_.clearLog();
	
			taskPool_.flush();
			taskPool_.fill(simulationTime);
			


			for (auto& node : nodes_)
				node.set_current_time(taskPool_.getSimulationStartTime());


			double nextAT = taskPool_.getTime(); //nextArrivalTime



			Task* arrived_task = &taskPool_.dequeue_task();


			//	cout<< "5" << endl;
			printArrive(*arrived_task);
			//	cout<< "6" << endl;
			run_scheduler(nextAT, *arrived_task);
			//	cout<< "7" << endl;

			int count = 0;

			//이제 시뮬레이터 종료까지 계속 스케줄러와 CPU를 실행한다.
			while (!taskPool_.isEnd()) {

				count++;

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
							node.execute(nextAT, logger_);
							node.set_arrival_time_reached(true);
						}
						else {
						
							node.execute(nextAT, logger_);
							/*}*/
						}
					}


				

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
					node.set_arrival_time_reached(false);
				}
			}

		

			logger_.createLogFile(scheduler_.get_algorithm_name());

			print_result();
		}
		else if (input == 3)
		{
			cout << "end simulation." << endl;
			return;
		}
	}

}


void Simulation::run_scheduler(double currentTime, Task& arrivedTask)
{
	scheduler_.run(currentTime, nodes_, arrivedTask);
}
void Simulation::print_intro()
{
	system("cls");

	cout << "---------------------------------------------------------------------" << endl;
	cout << "                   Welcome to The Server Simulator                   " << endl;
	cout << "---------------------------------------------------------------------" << endl;
	cout << endl;
}

void Simulation::print_menu(int select)
{

	switch (select)
	{
	case MAIN:
		cout << "Menu" << endl;
		cout << "1. Set simulation environment" << endl;
		cout << "2. Run simulation" << endl;
		cout << "3. End simulation" << endl;
		cout << endl;

		cout << "Select Menu : ";
		break;
	case SET_ENV:
		cout << "Menu" << endl;
		cout << "1. Set simulation time" << endl;
		cout << "2. Set core setting" << endl;
		cout << "3. Set Algorithm" << endl;
		cout << "4. End setting environment" << endl;
		cout << endl;
		break;
	case SET_CORE:
		break;
	case SET_ALGO:
		cout << "Algorithm Selection" << endl;
		cout << "1. random" << endl;
		cout << "2. MCT" << endl;
		cout << "3. algorithm 3" << endl;
		cout << "4. algorithm 4" << endl;
		cout << "5. algoirhtm 5 " << endl;
		cout << endl;

		break;
	}
}


void Simulation::set_simulation_environment() 
{
	int input = 0;
	int assignedNode = 0;
	int temp = 0;
	int coreType = 0;
	int algo = 0;


	while (1)
	{
		print_menu(SET_ENV);
		cin >> input;



		switch (input)
		{
		case SET_TIME:
			cout << "Set Simulation time : ";
			cin >> simulationTime;
			cout << "Simulation time set to : " << simulationTime << endl;
			break;
		case SET_CORE:

			nodes_.clear();
			
			cout << "Set Number of cores : ";
			cin >> numNode;
			
			cout << assignedNode << "nodes are assigned core numbers out of " << numNode << endl;
			cout << endl;

			while (assignedNode < numNode)
			{
				cout << "number of cores (maximum " << numNode - assignedNode << ") : ";
				cin >> temp;
				cout << "Select node type for " << temp << " cores (0-1) : ";
				cin >> coreType;
				for (int ii = assignedNode; ii < assignedNode + temp; ii++)
				{
					// create nodes and assign node types
					// node creator
					// set node type
					cout << assignedNode << " ";
				}
				cout << endl;
				cout << temp << " cores are set to be type " << coreType << endl;;
				assignedNode += temp;
			}

			break;
		case SET_ALGO:
			print_menu(SET_ALGO);
			cin >> algo;
			// algo 값을 필요한 곳에 저장해서 그걸로 알고리즘을 돌려야함
			switch (algo)
			{
			case 1:
				cout << "scheduler set to use random algorithm" << endl;
				break;
			case 2:
				cout << "scheduler set to use MCT algorithm" << endl;

				break;
			case 3:
				cout << "scheduler set to use algorithm 3" << endl;

				break;
			case 4:
				cout << "scheduler set to use algorithm 4" << endl;

				break;
			case 5:
				cout << "scheduler set to use algorithm 5" << endl;
				;

				break;
			}
			break;
		case 4:
			cout << "Finished setting envirionment" << endl;
			return;
			break;
		}

	}
}
void Simulation::print_result()
{
	cout << "Printing result" << endl;
	
	cout << "make span" << endl;
	cout << "execution success rate " << endl;
	cout << "cput utilization " << endl;
	// 다른 기타등등 결과
	
	cout << endl;


}