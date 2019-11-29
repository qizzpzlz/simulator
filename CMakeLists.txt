# CMakeList.txt : CMake project for cluster-simulator-cmake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project (cluster-simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(NOT MSVC)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
   if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
   endif()
endif(NOT MSVC)

# Build spdlog
find_package(Threads REQUIRED)
add_library(spdlog STATIC dependencies/spdlog.cpp)
target_compile_definitions(spdlog PUBLIC SPDLOG_COMPILED_LIB)
target_include_directories(spdlog PUBLIC dependencies/)
target_include_directories(spdlog PUBLIC dependencies/spdlog/)
target_include_directories(spdlog PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(spdlog PUBLIC Threads::Threads)

add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/bprinter dependencies)

add_library(argparse INTERFACE)
target_sources(argparse INTERFACE ${CMAKE_SOURCE_DIR}/dependencies/argparse.hpp)
target_include_directories(argparse INTERFACE dependencies/)

file(GLOB SRC_FILES
	./*.cpp
	src/*.cpp
	dependencies/*.cpp
	# dependencies/*.hpp
	./*.h
	includes/*.h
	dependencies/*.h)

# Add source to this project's executable.
add_executable (cluster-simulator ${SRC_FILES})

# OpenMP
find_package(OpenMP REQUIRED)
if (OpenMP_CXX_FOUND)
	target_link_libraries(cluster-simulator OpenMP::OpenMP_CXX)
endif()

# Link external libraries to the executable.
target_link_libraries(cluster-simulator spdlog)
target_link_libraries(cluster-simulator bprinter)
target_link_libraries(cluster-simulator argparse)

target_include_directories(cluster-simulator PUBLIC src/)
target_include_directories(cluster-simulator PUBLIC dependencies/)
target_include_directories(cluster-simulator PUBLIC dependencies/bprinter)
target_include_directories(cluster-simulator PUBLIC includes/)
target_include_directories(cluster-simulator PUBLIC scenarios/)

# TODO: Add tests and install targets if needed.

# Export compile commands for semantic completion.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
