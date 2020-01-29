project(static-genetic-algorithm)

set(CMAKE_CXX_STANDARD 17)

add_executable(static-genetic-algorithm
	main.cpp
	include/file_reader.h
	include/host.h
	include/job.h
	include/parameters.h
	include/chromosome.h
	include/genetic_algorithm.h
	src/file_reader.cpp
	src/chromosome.cpp
	src/genetic_algorithm.cpp
)
target_include_directories(static-genetic-algorithm PRIVATE include/)
target_link_libraries(static-genetic-algorithm json11)

find_package(OpenMP REQUIRED)
if (OpenMP_CXX_FOUND)
	target_link_libraries(static-genetic-algorithm OpenMP::OpenMP_CXX)
endif()