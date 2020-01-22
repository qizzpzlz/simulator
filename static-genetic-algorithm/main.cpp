#include "file_reader.h"
#include <filesystem>
#include <iostream>


int main(int argc, char* argv[])
{
	using namespace std::filesystem;

	auto current = current_path();

	std::cout << current;
	
	
	auto entries = read_binary("../static-genetic-algorithm/job-eligibility.small.bin");

	
}
