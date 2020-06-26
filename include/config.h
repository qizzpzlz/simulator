#pragma once
#include <string_view>
#include <chrono>

namespace cs::config
{
	using std::chrono::milliseconds;

	/**
	 * Compile-time configs
	 */
	
	static constexpr std::string_view SCENARIO_FILE_PATH_STRING =
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		"../scenarios/";
#else
		"scenarios/";
#endif
	static constexpr std::string_view JSON_HOST_FILE_NAME = "hardware_raw_initial_status.json";
	static constexpr std::string_view JSON_SCENARIO_FILE_NAME = "scenario.json";
	static constexpr std::string_view LOG_DIR_PATH_STRING = "logs/";

	/*Static settings for simulation.*/
	static constexpr std::string_view LOG_DIRECTORY = "logs";
	static constexpr std::string_view LOG_OUTPUT_FILE_NAME = "log_output.txt";
	static constexpr std::string_view JOBMART_FILE_NAME = "jobmart_raw_replica.txt";
	static constexpr std::string_view PERFORMANCE_FILE_NAME = "performance.txt";
	static constexpr std::string_view PENDING_FILE_NAME = "pending.txt";
	static constexpr std::string_view JOB_SUBMIT_FILE_NAME = "job_submit.txt";
	static constexpr std::string_view ALLOCATION_BINARY_FILE_NAME = "alloc.bin";
	
	static constexpr bool CONSOLE_OUTPUT = false;
	static constexpr bool CONSOLE_WARNING_OUTPUT = false;
	static constexpr bool LOG_FILE_OUTPUT = true;
	static constexpr bool JOBMART_FILE_OUTPUT = false;
	static constexpr bool SLOTS_FILE_OUTPUT = false;
	static constexpr bool JOB_SUBMIT_FILE_OUTPUT = false;
	static constexpr char LOGGER_PATTERN[] = "[%l] %v";
	static constexpr milliseconds DISPATCH_FREQUENCY{ 1000 };
	static constexpr milliseconds LOGGING_FREQUENCY{ 10000 };
	static constexpr milliseconds COUNTING_FREQUENCY{ 10000 };
	static constexpr bool USE_ONLY_DEFAULT_QUEUE = true;
	static constexpr double RUNTIME_MULTIPLIER = 1;
	static constexpr bool USE_STATIC_HOST_TABLE_FOR_JOBS = true;

	static constexpr bool LOG_ALLOCATION = true;

	static constexpr bool DEBUG_EVENTS = false;

}
