#pragma once
#include "Job.h"
#include "EnumConverter.h"
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

	class Host
	{
	public:
		std::string name;
		// Speed of the host's CPU relative to other hosts in the cluster.
		int cpu_factor;
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

		HostStatus status;
		int num_current_jobs{ 0 };
		int num_current_running_slots{ 0 };
		bool is_available_at_least_once{ false };

		
		//constexpr int score() const { return max_slot - num_current_running_slots + max_mem + nprocs + max_swp; }
		//int host_score = rand()%10000;
		int score() const noexcept { return score_; }

		constexpr bool is_executable(const Job& job) const
		{
			return status == HostStatus::OK
				&& job.slot_required + num_current_running_slots <= max_slot
				&& job.mem_required <= max_mem
				&& job.num_exec_procs <= nprocs * ncores;
				//&& job.swap_usage < max_swp;
		}

		// Status mutator methods
		void execute_job(const Job& job);
		void exit_job(const Job& job);
		void set_status(HostStatus value) noexcept
		{
			status = value;
			if (!is_available_at_least_once && value == HostStatus::OK)
				is_available_at_least_once = true;
		}
		void set_rand_score() noexcept { score_ = dist_(gen_); }

		// Initialise Host from status data.
		Host(const std::string name, int cpu_factor, int ncpus, int nprocs, int ncores, int nthreads, int max_slot, int max_mem, int max_swp,
			int max_tmp, HostStatus status)
			: name(name),
			  cpu_factor(cpu_factor),
			  ncpus(ncpus),
			  nprocs(nprocs),
			  ncores(ncores),
			  nthreads(nthreads),
			  max_slot(max_slot),
			  max_mem(max_mem),
			  max_swp(max_swp),
			  max_tmp(max_tmp),
			  status(status)
		{
			if (status == HostStatus::OK)
				is_available_at_least_once = true;
		}

	private:
		int slot_running_{};
		int score_{};

		static int id_gen_;
		static std::random_device rd_;
		static std::mt19937 gen_;
		static std::uniform_int_distribution<> dist_;
		inline static const int MAX_RAND_NUM = 100;
	};
}


//template<> const std::vector<std::string> Utils::enum_strings<ClusterSimulator::HostStatus>::data;

