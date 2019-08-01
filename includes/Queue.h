#pragma once
#include <map>
#include <vector>
#include <string>
#include <queue>
#include <functional>
#include "job.h"
#include "host.h"
#include "user.h"
//#include "../includes/limit.h"

namespace ClusterSimulator
{
	class ClusterSimulation;
	class Cluster;
	class Limit;

	class Queue
	{
	public:
		std::string name;
		int id{ id_gen_++ };
		int priority;

		//Queue params
		
		/**
		 * List of queue administrators (username or userGroup)
		 * Default : not defined
		 */
		std::vector<User> admin;
		
		/**
		 * The per-process (hard) core file size limit (in KB) 
		 * for all of the processes that belong to a job from this queue
		 * Default : unlimited
		 */
		//int core_limit{ -1 }

		/**
		 * Limits the total CPU time the job can use.
		 * Default : unlimited
		 */
		std::chrono::milliseconds cpu_limit{ -1 };

		/**
		 * EXCLUSIVE=Y | N 
		 * Jobs that are submitted to an exclusive queue with bsub -x are only dispatched to a host that has no other running jobs. 
		 */
		bool is_exclusive{ true };
		
		/**
		 * Fairshare
		 * default = not defined
		 * FAIRSHARE=USER_SHARES[[user, number_shares] ...]
		 */
		class User_shares
		{
			User user;
			int num_shares;
		};
		std::vector<User_shares> fairshare;
		bool is_using_fairshare() const noexcept { return !fairshare.empty(); }

		/**
		 * ex)
		 * HJOB_LIMIT = 1 
		 * HOSTS=hostA hostB hostC 
		 * 
		 * Maximum number of job slots that this queue can use on any host.
		 * This limit is configured per host, regardless of the number of processors it might have.
		 * default = unlimited
		 */
		//To-Do : 맵 
		// class Hjob_limit
		// {
		// 	int max_limit{ -1 };
		// 	//std::vector<Host> hosts;
		// 	Host host;
		// };
		// std::vector<Hjob_limit> hjob_limit;
		struct HostInfo
		{
			int slot_dispatched;
		};
		std::map<const Host*, HostInfo> dispatched_hosts_;

		std::vector<Limit*> limits;

		int using_job_slots() const noexcept;
		
		//int hjob_limit{ -1 };
		
		//The number you specify is multiplied by the value of lsb.params MBD_SLEEP_TIME (60 seconds by default).
		int job_accept_interval;

		//Total number of job slots that this queue can use
		int qjob_limit{ -1 };

		//requeue_exit_value : Enables automatic job requeue and sets the LSB_EXIT_REQUEUE environment variable.
		//REQUEUE_EXIT_VALUES=[exit_code ...] [EXCLUDE(exit_code ...)]

		//RESOURCE_RESERVE=MAX_RESERVE_TIME[integer]
		//Enables processor reservation and memory reservation for pending jobs for the queue

		//Resource requirements used to determine eligible hosts
		//res_req

		//The name of a host or host model specifies the runtime normalization host to use.
		std::chrono::milliseconds run_limit;

		//The per-process (hard) stack segment size limit (in KB) for all of the processes belonging to a job from this queue
		//int stack_limit;
		
		// Maximum number of job slots that each user can use in this queue.
		int ujob_limit{ -1 };

		/**
		 * users : A space-separated list of user names or user groups that can submit jobs to the queue
		 * default = all
		 * USERS=all [~user_name ...] [~user_group ...] | [user_name ...] [user_group [~user_group ...] ...]
		 */
		std::vector<User> users;
		
		/// Creates a default queue.
		explicit Queue(ClusterSimulation& simulation);
		/// Creates a custom queue with custom priority and name.
		Queue(ClusterSimulation& simulation, int priority, std::string name = std::string());
		Queue(ClusterSimulation& simulation, const std::string& name);
		~Queue();

		bool operator<(const Queue& other) const noexcept { return priority < other.priority; }

		/// Gets priority of this queue. Higher values have higher priority.
		constexpr int get_priority() const { return priority; }
		int count() const { return jobs_.size(); }
		constexpr bool is_default() const { return is_default_; }

		bool dispatch();
		void enqueue(Job&& job);

	private:
		using HostReference = Host*;
		using HostList = std::vector<HostReference>;
		HostList match(const Job& job);
		void sort(HostList::iterator first, HostList::iterator last);
		void policy();
		void clean_pending_jobs();
		void set_compare_host_function_(const std::function<bool(const HostReference, const HostReference)> compare_host) noexcept
		{
			compare_host_function_ = compare_host;
		}

		// characteristics
		bool is_default_{};
		int default_host_specification_{};

		// fields
		ClusterSimulation* simulation_;
		std::vector<Job> jobs_;
        // TODO: maybe unnecessary
        std::vector<Job> pending_jobs_;
		std::vector<Job> running_jobs_;
		std::vector<Job> suspended_jobs_;

		// Queue limits
		int job_limit_{};
		int job_limit_per_processor_{};
		// UNIX and Linux limits
		// Policies
		// Administrators
		// Run conditions
		// Load-sharing threshold conditions
		// UNIX nice value
		// Queue states
		
		// Restrictions
		// Restrict host
		// Restrict job size

		std::function<bool(const HostReference, const HostReference)> compare_host_function_
		{
			[](const HostReference a, const HostReference b)
			{	
				return a->score() < b->score();
			}
		};

		// Static fields
		static int id_gen_;
		static const int DEFAULT_PRIORITY = 40;
		
		class CompareJob
		{
		public:
			bool operator() (const Job& a, const Job& b)
			{
				return a.priority > b.priority;
			}
		};

		class StaticQueueData
		{
		public:
			//static std::map<std::vector<std::string>, int> queue_priorities_vectors;

			static std::map<std::string, int> queue_priorities;

			StaticQueueData()
			{
				std::map<std::vector<std::string>, int> queue_priorities_vectors =
				{
					{std::vector<std::string>{"normal"}, 30},
					{std::vector<std::string>{"icc_mem", "ostest", "ostest_finesim", "userdefine", "userdefine_rent"}, 100},
					{std::vector<std::string>{"techgen"}, 150},
					{std::vector<std::string>{"caldfm_bigmem", "caldfm_gpu","caldfmrve", "calstar_mem", "dfmprism", "dpat", "fatdown_res", "hds", "hds_com", "hpg", "nwave", "ora", "pgsend2", "pmhsl", "slsidfm", "slsidp","slsipm", "srt_long", "tessent_dedicate"}, 200},
					{std::vector<std::string>{"grdgen"}, 240},
					{std::vector<std::string>{"adv","med", "redhawk_dmp_ssir_idt", "redhawk_ssir_idt", "zebu_com", "zebu_com_1u_48", "zebu_com_eval", "zebu_com_pv"}, 250},
					{std::vector<std::string>{"hfss_lsi", "hfss_mem", "mtlong"}, 270},
					{std::vector<std::string>{"mtmed"}, 280},
					{std::vector<std::string>{"fsspice_long", "mtshort"}, 290},
					{std::vector<std::string>{"brionorcmem", "calmdf", "calmdp_adv", "calview", "calview_foundry", "calview_foundry_long", "calview_kiwi", "calview_kiwi_long", "calview_long", "ccp", "ciov", "dsg-compiler", "ead", "encounter", "encounter_asic", "fsdp", "fspro_tat", "fsrmt","fsspice", "gtb", "gui", "icc", "icc_dedicate", "icc_mt", "icc_snps", "icc_ssir_idt", "jasper_long", "kiwi", "kiwi_mp", "lcalwb_model", "lcalwb_view", "long", "mig","nanospice", "nanospice_long","nanospice_scm",
			"nanotime","perf_long", "plong","primetime_long", "pxp_com",
			"pxp_power", "railadvisor2", "redhawk", "redhawk_asic",
			"redhawk_dmp", "redhawk_fdry1", "redhawk_idt1", "redhawk_pjx", "redhawk_pjx2", "redhawk_pjx3","redhawk_sc", "slsimdp", "slsiopc", "spectre", "spectre_aps", "star-rcxt", "star-rcxt_ap", "star-rcxt_asic", "tetramax", "tetramax_dedicate",
			"tflexview", "vlc_com", "voltus_dp","zebu_pnr",
			}, 300},
					{std::vector<std::string>{"lcalwb_auto", "short", "tetramax_short"}, 310},
					{std::vector<std::string>{"pmed"}, 340},
					{std::vector<std::string>{"fsscope", "fsscv", "pgui", "pshort", "spectre_med", "totem_large", "tpa"}, 350},
					{std::vector<std::string>{"fsp_cm", "fss_cm"}, 390},
					{std::vector<std::string>{"ccp_extraction", "dsg-compiler_short", "fspro", "fspro_dedicate", "fss", "fss_ip", "fsspice_dedicate", "fsspice_tat", "primetime", "primetime_prj", "saibis", "spectre_ams", "spectre_dedicate", "spectre_short", "spectre_ultra","star-rcxt_short"}, 400},
					{std::vector<std::string>{"fsspice_short"}, 410},
					{std::vector<std::string>{"caldrc_ssir_idt", "fsnano", "fss_short", "maskcheck_bigmem", "nanosim", "pmtlong", "verilog_debug", "verilog_regression"}, 450},
					{std::vector<std::string>{"pmtmed"}, 460},
					{std::vector<std::string>{"fsp", "fsp_ip"}, 470},
					{std::vector<std::string>{"verilog_multi"}, 480},
					{std::vector<std::string>{"hercdrc"}, 490},
					{std::vector<std::string>{"caldrc", "caletc", "callvs","calxact", "cdauto", "emul_post",
			"fsmcpu", "fsp_mt", "fss_mt", "herclvs", "icvdrc", "nanosim_short", "pmtshort", "primetime_short", "redhawk_mem", "slsilfd", "verilog_bigmem", "verilog_compile", "verilog_long", "verilog_short",
			}, 500},
					{std::vector<std::string>{"caldpt", "herclvs_short", "icvlvs"}, 510},
					{std::vector<std::string>{"icvlvs_short"}, 540},
					{std::vector<std::string>{"3dem", "aps", "ares2_caldrc", "ares2_callvs", "caldrc_short", "callvs_short", "hercdrc_short", "icvdrc_short", "nimbic", "powersi", "q3d", "siwave", "vcs", "vcs_bigmem", "verilog_vm"}, 550},
					{std::vector<std::string>{"magillem"}, 600},
					{std::vector<std::string>{"hs", "hspice"}, 640},
					{std::vector<std::string>{"hs_long", "hspice_long"}, 680},
					{std::vector<std::string>{"cosim_long", "fshbm", "fshbm8", "hs_med", "hspice_med"}, 700},
					{std::vector<std::string>{"hs_short", "hspice_scope", "hspice_short"}, 750},
				};

				for (const auto& vec : queue_priorities_vectors)
				{
					for (const auto& str : vec.first)
					{
						queue_priorities.insert(std::make_pair(str, vec.second));
					}
				}
			}
		};

		const static StaticQueueData data;
	};
}


