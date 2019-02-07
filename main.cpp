#include <iostream>
#include "Simulation.h"
#include "RandomAlgorithm.h"

int main()
{
	//test
	double simulationTime = 7200;
	int nodeNum = 8;
	RandomAlgorithm algorithm{};

	Simulation simulation{algorithm, simulationTime, nodeNum};

	simulation.simulate();
}

