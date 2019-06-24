#pragma once
#include <map>
#include <vector>
#include <string>
#include "Job.h"

namespace ClusterSimulator
{
	class ClusterSimulation;
	class Host;

	class Queue
	{
	public:
		//friend class Job;
		//using Policy = std::function<Host&(Queue& queue)>

		const std::string name;
		const int id{ id_gen_++ };
		const int priority;

		/// Creates a default queue.
		explicit Queue(ClusterSimulation& simulation);
		/// Creates a custom queue with custom priority and name.
		Queue(ClusterSimulation& simulation, int priority, std::string name = std::string());
		Queue(ClusterSimulation& simulation, const std::string& name);
		~Queue();

		/// Gets priority of this queue. Higher values have higher priority.
		int get_priority() const { return priority; }

		int count() const { return jobs_.size(); }
	
		bool is_default() const { return is_default_; }

		void enqueue(Job&& job);
	private:
		ClusterSimulation& simulation_;
		//Policy policy_ = queue.simple_default_policy;

		// characteristics
		bool is_default_{};

		int default_host_specification_{};

		// fields
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

		static int id_gen_;
		static const int DEFAULT_PRIORITY = 40;
		
		Host& simple_default_policy(Queue& queue)
		{
			
		}

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


