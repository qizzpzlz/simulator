#include "file_reader.h"
#include <filesystem>
#include <iostream>
#include "genetic_algorithm.h"

int main(int argc, char* argv[])
{
	//using namespace std::filesystem;

	//auto current = current_path();

	//std::cout << current;

	//std::cout << json11::Json::BOOL;
	

	//genetic::read_host_json("../static-genetic-algorithm/all-time-ok-hosts.json", "hosts.bin");

	//genetic::read_host_binary();
	
	//auto entries = genetic::read_binary("../static-genetic-algorithm/job-eligibility.small.bin");
	

	genetic::generate_initial_population();
}
