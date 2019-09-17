#pragma once
#include "job.h"
#include "enum_converter.h"
#include <string>
#include <vector>
#include <random>

namespace ClusterSimulator
{
	enum class HostStatus
	{
		OK,
		Closed_Adm,
		Closed_Busy,
		Closed_Excl,
		Closed_cu_Excl,
		Closed_Full,
		Closed_LIM,
		Closed_Lock,
		Closed_Wind,
		Closed_EGO,
		Unavail,
		Unreach
	};

	class Cluster;
	class ClusterSimulation;

	class Host
	{
	public:
		// Speed of the host's CPU relative to other hosts in the cluster.
		double cpu_factor;
		// Number of CPUs you have specified for your host.
		int ncpus;
		// Number of physical processors.
		int nprocs;
		// Number of cores per processors.
		int ncores;
		int nthreads;
		int max_slot;
		int max_mem;
		int max_swp;
		int max_tmp;
		int id{ id_gen_++ };

		const Cluster* cluster;

		HostStatus status;
		int num_current_jobs{ 0 };
		int num_current_running_slots{ 0 };
		bool is_available_at_least_once{ false };

		const std::string& get_name() const noexcept{ return name_; }
		constexpr int score() const noexcept { return score_; }
		constexpr int remaining_slots() const noexcept { return max_slot - num_current_running_slots; }
		constexpr bool is_executable(const Job& job) const noexcept
		{
			return status == HostStatus::OK
				&& job.slot_required <= remaining_slots()
				&& job.mem_required <= max_mem
				&& job.num_exec_procs <= nprocs * ncores;
				//&& job.swap_usage < max_swp;
		}
		std::chrono::milliseconds get_expected_run_time(const Job& job) const noexcept;
		ms get_expected_time_of_all_completion() const noexcept { return expected_time_of_completion; }


		/* Status mutator methods */

		void execute_job(const Job& job);
		void exit_job(const Job& job);
		void set_status(HostStatus value) noexcept
		{
			status = value;
			if (!is_available_at_least_once && value == HostStatus::OK)
				is_available_at_least_once = true;
		}
		void set_rand_score() noexcept { score_ = dist_(gen_); }
		void try_update_expected_time_of_completion(std::chrono::milliseconds run_time) noexcept;

		// Initialise Host from status data.
		Host(const std::string& name, double cpu_factor, int ncpus, int nprocs, int ncores, int nthreads, int max_slot, int max_mem, int max_swp,
			int max_tmp, const std::string& host_group, HostStatus status, const Cluster& cluster)
			: name_(name),
			  host_group_(host_group),
			  cpu_factor(cpu_factor),
			  ncpus(ncpus),
			  nprocs(nprocs),
			  ncores(ncores),
			  nthreads(nthreads),
			  max_slot(max_slot),
			  max_mem(max_mem),
			  max_swp(max_swp),
			  max_tmp(max_tmp),
			  cluster(&cluster),
			  status(status)
		{
			if (status == HostStatus::OK)
				is_available_at_least_once = true;
		}

	private:
		std::string name_;
		std::string host_group_;
		int slot_running_{};
		int score_{};
		ms expected_time_of_completion{};

		// double max_procs;

		static int id_gen_;
		static std::random_device rd_;
		static std::mt19937 gen_;
		static std::uniform_int_distribution<> dist_;
		inline static const int MAX_RAND_NUM = 100;
	};
}


//template<> const std::vector<std::string> Utils::enum_strings<ClusterSimulator::HostStatus>::data;

