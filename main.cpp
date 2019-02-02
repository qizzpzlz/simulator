#include <iostream>
#include "Simulation.h"
#include "RandomAlgorithm.h"

int main()
{
	double simulationTime = 1;
	int nodeNum = 8;
	RandomAlgorithm algorithm{};

	Simulation simulation{algorithm, simulationTime, nodeNum};

	simulation.simulate();
}

