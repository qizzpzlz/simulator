#include "Task.h"
#include <sstream>
//#include <iomanip>

using namespace std;

std::string Task::print()
{
	std::stringstream sstream;

	//sstream << "task#|task ID|Node ID|Start Time|Excution Time|Finished Time"<<endl;
	if (id == 0)
	{
		sstream << "Task ID : " << setw(6) << id << "  Node ID : " << setprecision(5) << get_executed_core_id();
		sstream << "  Start Time : " << setprecision(6) << "0.0000";
		sstream << " Excution Time : " << setprecision(6) << get_execution_time();
		sstream << "  Finished Time : " << setprecision(6) << get_finished_time();
		if (state_ == TaskState::Completed)
		{
			sstream << "  Task state : Completed" << endl;
		}
		else if (state_ == TaskState::Running)
		{
			sstream << "  Task state : Running" << endl;
		}
		else if (state_ == TaskState::Unexecuted)
		{
			sstream << "  Task state : Unexcuted" << endl;
		}
		
	}
	else
	{
		sstream << "Task ID : " << setw(6) << id << "  Node ID : " << setprecision(5) << get_executed_core_id();
		sstream << "  Start Time : " << setw(6) << get_start_time();
		sstream << "  Excution Time : " << setw(6) << get_execution_time();
		sstream << "  Finished Time : " << setw(6) << get_finished_time();
		if (state_ == TaskState::Completed)
		{
			sstream << "  Task state : Completed" << endl;
		}
		else if (state_ == TaskState::Running)
		{
			sstream << "  Task state : Running" << endl;
		}
		else if (state_ == TaskState::Unexecuted)
		{
			sstream << "  Task state : Unexcuted" << endl;
		}
	}
	// ...

	return sstream.str();

}
