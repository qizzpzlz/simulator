#pragma once
#include <map>
#include <chrono>
namespace cs
{
    class Queue;
    class Host;
    class Job;

    class Limit
    {
    public:
	    virtual ~Limit() = default;
	    virtual bool is_eligible(const Queue& queue, const Host& host, const Job& job) const = 0;
    };

	//The name of a host or host model specifies the runtime normalization host to use.
    class RuntimeLimit : public Limit
    {
        std::map<const Host*, std::chrono::milliseconds> entries_;
    public:
        bool is_eligible(const Queue& queue, const Host& host, const Job& job) const override;
    };

	/**
	 * ex)
	 * HJOB_LIMIT = 1
	 * HOSTS=hostA hostB hostC
	 *
	 * Maximum number of job slots that this queue can use on any host.
	 * This limit is configured per host, regardless of the number of processors it might have.
	 * default = unlimited
	 */
    class HjobLimit : public Limit
    {
        std::map<const Host*, int> entries_;
    public:
        bool is_eligible(const Queue& queue, const Host& host, const Job& job) const override;
    };

	/**
	* EXCLUSIVE=Y | N
	* Jobs that are submitted to an exclusive queue with bsub -x are only dispatched to a host that has no other running jobs.
	*/
	class ExclusiveLimit : public Limit
	{
	public:
		bool is_eligible(const Queue& queue, const Host& host, const Job& job) const override;
	};
}