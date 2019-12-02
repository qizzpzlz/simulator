#include "parser.h"
#include "scenario.h"
#include "cluster.h"
#include "enum_converter.h"
#include "json11.hpp"
#include <fstream>
#include <iostream>
#include <sstream>


namespace ClusterSimulator::Parser
{
		void parse_scenario(Scenario* scenario, const std::string& file_path, int limit)
		{
			using namespace json11;

			std::ifstream fin;
			fin.open(file_path);

			if (!fin)
				throw std::runtime_error("File couldn't be opened.");

			std::cout << "Parsing scenario file..." << std::endl;

			std::string line;
			std::string err;
			bool flag{false};
			ms initial_timestamp;
			int count{ 0 };
			while (std::getline(fin, line))
			{
				Json json = Json::parse(line, err);
				const Json& details = json["event_detail"];
				// if (details["exec_hostname"].string_value().empty())
				// 	continue;

				ScenarioEntry entry;
				if (!flag)
				{
					flag = true;
					initial_timestamp = ms{ seconds{json["event_timestamp"].int_value()} };
					scenario->initial_time_point = ms{};
				}

				entry.timestamp = ms(ms{ seconds{ json["event_timestamp"].int_value() } } - initial_timestamp);
				if (entry.timestamp < ms{})
				{
					throw std::runtime_error("Scenario is not ordered properly.");
				}
					
				if (json["event_action"] == "submission")
				{
					entry.type = ScenarioEntry::ScenarioEntryType::SUBMISSION;
					entry.event_detail.queue_name = details["queue_name"].string_value();
					entry.event_detail.exec_hostname = details["exec_hostname"].string_value();
					entry.event_detail.application_name = details["application_name"].string_value();
					entry.event_detail.job_id = details["job_id"].int_value();
					entry.event_detail.mem_req = details["mem_req"].int_value();
					entry.event_detail.number_of_jobs = details["number_of_jobs"].int_value();
					entry.event_detail.num_exec_procs = details["num_exec_procs"].number_value();
					entry.event_detail.num_slots = details["num_slots"].int_value();
					entry.event_detail.job_mem_usage = details["job_mem_usage"].int_value();
					entry.event_detail.job_swap_usage = details["job_swap_usage"].int_value();
					entry.event_detail.job_cpu_time = details["job_cpu_time"].int_value();
					entry.event_detail.job_run_time = details["job_run_time"].int_value();
					entry.event_detail.job_exit_status = details["job_exit_status"].string_value();
					entry.event_detail.job_exit_code = details["job_exit_code"].int_value();
					entry.event_detail.job_non_cpu_time = details["job_non_cpu_time"].int_value();
					entry.is_multi_host_submission = json["MultiHost"].bool_value();

					if (!entry.is_multi_host_submission && entry.event_detail.num_exec_procs > entry.event_detail.num_slots)
					{
						entry.is_multi_host_submission = true;
					}
				}
				else
				{
					entry.type = ScenarioEntry::ScenarioEntryType::CHANGE_STATUS;
					entry.event_detail.host_name = details["host_name"].string_value();
					HostStatus status;
					std::stringstream ss(details["host_status"].string_value());
					ss >> Utils::enum_from_string<HostStatus>(status);
					entry.event_detail.host_status = status;
					entry.event_detail.cpu_factor = details["NCPUS"].number_value();
					entry.event_detail.ncpus = details["NCPUS"].int_value();
					entry.event_detail.nprocs = details["NPROCS"].int_value();
					entry.event_detail.ncores = details["NCORES"].int_value();
					entry.event_detail.nthreads = details["NTHREADS"].int_value();
				}

				scenario->add_scenario_entry(entry);

				if (count++ == limit)
					break;
			}

			if (scenario->count() == 0)
				throw std::runtime_error("Can't read the scenario file.");

			std::cout << "Successfully parsed " << scenario->count() << " scenario entries." << std::endl;
		}

		void parse_cluster(Cluster* cluster, const std::string& file_path)
		{
			using namespace json11;
			std::ifstream fin;
			fin.open(file_path);

			if (!fin)
			{
				std::cerr << "File couldn't be opened.";
			}

			std::cout << "Parsing hosts file..." << std::endl;

			const std::string str((std::istreambuf_iterator<char>(fin)),
				std::istreambuf_iterator<char>());

			std::string err;
			const Json json = Json::parse(str, err);

			std::cout << json.string_value();

			std::map<std::string, Json> map = json.object_items();
			for (auto const& pair : map)
			{
				const Json& detail = pair.second;

				HostStatus status;
				std::stringstream ss(detail["HOST_STATUS"].string_value());
				ss >> Utils::enum_from_string<HostStatus>(status);

				cluster->add_node(Host(pair.first,
					detail["CPU_FACTOR"].number_value(),
					detail["NCPUS"].int_value(),
					detail["NPROCS"].int_value(),
					detail["NCORES"].int_value(),
					detail["NTHREADS"].int_value(),
					detail["MAX_SLOT"].int_value(),
					detail["MAX_MEM"].int_value(),
					detail["MAX_SWP"].int_value(),
					detail["MAX_TMP"].int_value(),
					detail["HOST_GROUP"].string_value(),
					status, *cluster));
			}

			std::cout << "Successfully parsed a cluster with " << cluster->count() << " hosts." << std::endl;
		}
}
