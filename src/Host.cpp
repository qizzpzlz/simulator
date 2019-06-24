#include "../includes/Job.h"
#include "../includes/Host.h"
#include "../dependencies/spdlog/spdlog.h"

namespace ClusterSimulator
{
	int Host::id_gen_ = 0;
	void Host::execute_job(const Job& job)
	{
		if (slot_running_ + job.slot_required > max_slot)
			spdlog::error("Host {0}: Slot required for job {1} cannot be fulfilled with this host.", id, job.id);

		slot_running_ += job.slot_required;
	}

	void Host::exit_job(const Job& job)
	{
		slot_running_ -= job.slot_required;
	}
}