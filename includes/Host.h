#pragma once
#include "Job.h"
#include "EnumConverter.h"
#include <string>
#include <vector>

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
		const std::string name;
		const int cpu_factor;
		const int ncpus;
		const int nprocs;
		const int ncores;
		const int nthreads;
		const int max_slot;
		const int max_mem;
		const int max_swp;
		const int max_tmp;
		const int id{ id_gen_++ };

		constexpr int score() const { return max_slot - num_current_running_slots + max_mem + nprocs + max_swp; }
		constexpr bool is_executable(const Job& job) const
		{
			return status == HostStatus::OK
				&& job.slot_required + num_current_running_slots < max_slot
				&& job.mem_required < max_mem
				&& job.num_exec_procs < nprocs
				&& job.swap_usage < max_swp;
		}

		HostStatus status;
		int num_current_jobs{ 0 };
		int num_current_running_slots{ 0 };
		bool is_available_at_least_once{ false };

		// Status mutator methods
		void execute_job(const Job& job);
		void exit_job(const Job& job);
		void set_status(HostStatus value) 
		{
			status = value;
			if (!is_available_at_least_once && value == HostStatus::OK)
				is_available_at_least_once = true;
		}

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
		int slot_running_{ 0 };

		static int id_gen_;
	};
}


//template<> const std::vector<std::string> Utils::enum_strings<ClusterSimulator::HostStatus>::data;

