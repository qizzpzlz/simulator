# CMakeList.txt : CMake project for cluster-simulator-cmake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project (cluster-simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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


file(GLOB SRC_FILES
	./*.cpp
	src/*.cpp
	dependencies/*.cpp
	./*.h
	includes/*.h
	dependencies/*.h)

# Add source to this project's executable.
add_executable (cluster-simulator ${SRC_FILES})

target_link_libraries(cluster-simulator spdlog)

target_include_directories(cluster-simulator PUBLIC src/)
target_include_directories(cluster-simulator PUBLIC dependencies/)
target_include_directories(cluster-simulator PUBLIC includes/)
target_include_directories(cluster-simulator PUBLIC scenarios/)

# TODO: Add tests and install targets if needed.
