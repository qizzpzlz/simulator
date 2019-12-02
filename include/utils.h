#pragma once
#include <chrono>
#include <omp.h>

namespace Utils
{
    using namespace std::chrono;
    using ms = time_point<milliseconds>;

    constexpr milliseconds get_time_left_until_next_period(const ms current, const milliseconds frequency)
    {
        const double current_d{ static_cast<double>(current.time_since_epoch().count()) };
        const long long frequency_d{ frequency.count() };
        const double dividend{ current_d / frequency_d };
        const long long dividend_i{ static_cast<long long>(dividend) };

        //if (dividend_i == static_cast<double>(dividend_i))
        //    return std::chrono::milliseconds(1);

		return milliseconds{ (dividend_i/* + (dividend > 0 ? 1 : 0)*/) * frequency_d -
			current.time_since_epoch().count() };
    }

    struct ms_hash
    {
        std::size_t operator()(const ms& h) const
        {
            return std::hash<int64_t>{}(h.time_since_epoch().count());
        }
    };

	template <class T, class Pr>
	std::pair<unsigned, T> min_element_parallel(const std::vector<T>& vec, Pr predicate)
	{
		using IndexValuePair = std::pair<unsigned, T>;	// TODO: no copy for non-pointer types

		IndexValuePair minIndexValue{ 0, vec[0] };
		int i;
		auto size = vec.size();

		#pragma omp parallel
		{
			IndexValuePair min(minIndexValue);
			#pragma omp for nowait
			for (i = 1; i < size; i++)
				if (predicate(vec[i], min.second))
					min = IndexValuePair(i, vec[i]);
			#pragma omp critical
			{
				if (predicate(min.second, minIndexValue.second))
					minIndexValue = IndexValuePair(min);
			}
		}

		return minIndexValue;
	}

	template <class T>
	class MemoryPool
	{
		
	};
}