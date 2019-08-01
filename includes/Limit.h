#pragma once
#include <map>
#include <chrono>
namespace ClusterSimulator
{
    class Queue;
    class Host;
    class Job;

    class Limit
    {
    public:
        virtual bool is_eligible(const Queue& queue, const Host& host, const Job& job) const = 0;
    };
    class RuntimeLimit : public Limit
    {
        std::map<const Host*, std::chrono::milliseconds> entries_;
    public:
        bool is_eligible(const Queue& queue, const Host& host, const Job& job) const override;
    };
    class HjobLimit : public Limit
    {
        std::map<const Host*, int> entries_;
    public:
        bool is_eligible(const Queue& queue, const Host& host, const Job& job) const override;
    };
    class Exclusive : public Limit
    {
    public:
        bool is_eligible(const Queue& queue, const Host& host, const Job& job) const override;
    };

}