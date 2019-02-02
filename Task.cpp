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
		sstream << "Task ID : " << setw(5) << id << "  Node ID : " << setprecision(5) << get_executed_core_id();
		sstream << "  Start Time : " << setprecision(5) << "0.0000 ";
		sstream << " Excution Time : " << setprecision(5) << "0.0000 ";
		sstream << "  Finished Time : " << setprecision(5) << "0.0000 "<<endl;
	}
	else
	{
		sstream << "Task ID : " << setw(5) << id << "  Node ID : " << setprecision(5) << get_executed_core_id();
		sstream << "  Start Time : " << setprecision(5) << get_start_time();
		sstream << "  Excution Time : " << setprecision(5) << get_execution_time();
		sstream << "  Finished Time : " << setprecision(5) << get_finished_time() << endl;
	}
	// ...

	return sstream.str();

}
