#include "Node.h"
#include "Simulation.h"
using namespace std;

int Node::id_gen_ = 0;

NodeState Node::get_state() const
{
	return state_;
}

void Node::enqueue_task(Task& task)
{
	task.set_executed_core_id(Id);
	queue_.enqueue(task);
}

bool Node::try_set_paused_task()
{
	if (paused_queue_.is_empty())
		return false;
	set_task(paused_queue_.dequeue());
	return true;
}

bool Node::try_set_task_from_queue()
{
	if (queue_.is_empty())
		return false;
	set_task(queue_.dequeue());
	return true;
}

double Node:: execute(double current_time, Logger& logger)
{
	Task& task = current_task_;

	//상태 변환하는 부분
	if (state_ == NodeState::Idle) {
		//할 일 없음.
		return 0;
	}
	else if (task.get_state() == TaskState::Running) {

	


		//한 번 실행됬다가 남은 작업을 실행할 때 상태 전환
		//실행 시킬 수 있다면 실행상태로.
		/*
		cpu[core].startTime = cpuNowTime[core];
		cpu[core].state = Execute;
		printResume(cpu[core]);
		*/
		task.set_start_time(get_current_time());
		
	}

	if (current_time < current_time_ + task.leftTime) {
		//다음 도착, 종료예정시간까지 실행이 불가능하다면
		//다음 도착, 종료예정시간까지만 실행
		task.leftTime -= current_time - current_time_;
		task.set_execution_time(task.get_execution_time() + current_time - current_time_);
		current_time_ = current_time;
		//printf("4 : %f\n",now);
		//cpu[core].state = Execute;
	}
	else {
		//남은 시간 내에 완수한다면
		//실행하고 유휴상태로 전환
		current_time_ += task.leftTime;
		//cout << "5 : " << cpuNowTime[core] << endl;
		task.set_execution_time(task.get_execution_time() + task.leftTime);
		task.leftTime = 0;
		task.set_finished_time(current_time_);
		task.set_state(TaskState::Completed);
		//printCom(cpu[core]);
		logger.writeLog(task);
		state_ = NodeState::Idle;
		task.set_state(TaskState::Completed);
	}


	

	return exe_time_;

}

void Node:: set_current_time(const double time)
{
	current_time_ = time;
}

void Node::set_task(Task&& task)
{
	//current_task_ptr_ = &task;
	current_task_ = task;
	current_task_.set_state(TaskState::Running);
	//state_ = task.get_state();
	state_ = NodeState::Running;
	set_current_time(task.get_arrival_time());
}
Task& Node:: get_task()
{	
	// Unsafe
	return current_task_;
}
double Node::get_left_time() const
{
	if (state_ != NodeState::Idle) 
	{
		return left_time_;
	}
	else 
	{
		return 0;
	}

}
double Node::get_exe_time() const
{
	if (state_ != NodeState::Idle) {
		return exe_time_;
	}
	else {
		return 0;
	}

}


