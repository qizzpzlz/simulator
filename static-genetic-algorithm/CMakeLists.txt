project(static-genetic-algorithm)

set(CMAKE_CXX_STANDARD 17)

add_executable(static-genetic-algorithm
	main.cpp
	file_reader.h
	file_reader.cpp
	host.h
	job.h
	chromosome.h
	genetic_algorithm.h
	genetic_algorithm.cpp)

target_link_libraries(static-genetic-algorithm json11)