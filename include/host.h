#pragma once
#include "job.h"
#include <string>
#include <random>
#include <memory>
#include <utility>
#include <streambuf>

namespace cs
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

	struct HostInfo
	{
		float cpu_factor;
		unsigned char max_slots;

		bool write(std::streambuf& buf) const
		{
			const auto size = sizeof(HostInfo);
			return buf.sputn(reinterpret_cast<const char*>(this), size);
		}
	};

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
		HostStatus status;

		int num_current_running_slots{ 0 };
		bool is_available_at_least_once{ false };

		ClusterSimulation* simulation;
		Cluster* cluster;

		[[nodiscard]] const std::string& get_name() const noexcept{ return name_; }
		[[nodiscard]] int score() const noexcept { return score_; }
		[[nodiscard]] int remaining_slots() const noexcept { return max_slot - num_current_running_slots; }
		[[nodiscard]] std::size_t num_running_jobs() const noexcept { return running_jobs_.size(); }

		/**
		 * Returns true if this host's hardware spec satisfies
		 * the requirements imposed by a specified job.
		 */
		[[nodiscard]] bool is_compatible(const Job& job) const noexcept
		{
			return status == HostStatus::OK
				&& job.slot_required <= max_slot
				&& job.mem_required <= max_mem;	// TODO: Consider run-time memory usages.
			//	&& job.num_exec_procs <= nprocs * ncores;
			//	&& job.swap_usage < max_swp;
		}

		/**
		 * Returns true if this host is eligible for a specified job.
		 */
		[[nodiscard]] bool is_executable(const Job& job) const noexcept
		{
			return is_compatible(job)
				&& job.slot_required <= remaining_slots();
		}

		[[nodiscard]] milliseconds get_ready_duration(const Job& job) const;
		[[nodiscard]] milliseconds get_expected_completion_duration(const Job& job) const;
		[[nodiscard]] milliseconds get_expected_run_duration(const Job& job) const noexcept;
		[[nodiscard]] ms get_expected_time_of_all_completion() const noexcept { return expected_time_of_completion; }
		[[nodiscard]] int get_expected_remaining_slots(const Job& job) const;


		/* Status mutator methods */

		void execute_job(std::shared_ptr<Job>& job_ptr, bool reserved = false);
		void execute_job_when_ready(std::shared_ptr<Job>& job, milliseconds delay);
		void set_status(HostStatus value) noexcept
		{
			status = value;
			if (!is_available_at_least_once && value == HostStatus::OK)
				is_available_at_least_once = true;
		}
		void set_rand_score() noexcept { score_ = dist_(gen_); }
		void try_update_expected_time_of_completion(milliseconds run_time) noexcept;

		// Initialise Host from status data.
		Host(std::string name, double cpu_factor, int ncpus, int nprocs, int ncores, 
			 int nthreads, int max_slot, int max_mem, int max_swp, int max_tmp, 
			 std::string host_group, HostStatus status, Cluster& cluster)
			: cpu_factor(cpu_factor <= 0 ? 30 : cpu_factor),
			  ncpus(ncpus),
			  nprocs(nprocs),
			  ncores(ncores),
			  nthreads(nthreads),
			  max_slot(max_slot),
			  max_mem(max_mem),
			  max_swp(max_swp),
			  max_tmp(max_tmp),
			  status(status),
			  cluster(&cluster),
			  name_(std::move(name)),
			  host_group_(std::move(host_group))
		{
			if (status == HostStatus::OK)
				is_available_at_least_once = true;
		}

		Host(const Host&) = delete;
		Host operator=(const Host&) = delete;
		Host(Host&&) = default;
		Host& operator=(Host&&) = default;

	private:
		std::string name_;
		std::string host_group_;
		//int slot_running_{};
		int score_{};
		ms expected_time_of_completion{};

		// double max_procs;

		std::vector<std::shared_ptr<Job>> running_jobs_;
		//std::vector<std::pair<std::shared_ptr<Job>, std::size_t>> reserved_jobs_;

		void update_job_list(const std::shared_ptr<Job>& job_ptr);
		void exit_job(std::shared_ptr<Job> job_ptr, bool reserved = false);

		static int id_gen_;
		static std::random_device rd_;
		static std::mt19937 gen_;
		static std::uniform_int_distribution<> dist_;
		inline static const int MAX_RAND_NUM = 100;
	};
}


//template<> const std::vector<std::string> Utils::enum_strings<ClusterSimulator::HostStatus>::data;

