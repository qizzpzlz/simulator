#include <chrono>

namespace Utils
{
    constexpr std::chrono::milliseconds get_time_left_until_next_period(
        const std::chrono::time_point<std::chrono::milliseconds> current,
        const std::chrono::milliseconds frequency)
    {
        const double current_d{ static_cast<double>(current.time_since_epoch().count()) };
        const long long frequency_d{ frequency.count() };
        const double dividend{ current_d / frequency_d };
        const long long dividend_i{ static_cast<long long>(dividend) };

        //if (dividend_i == static_cast<double>(dividend_i))
        //    return std::chrono::milliseconds(1);

		return std::chrono::milliseconds{ (dividend_i/* + (dividend > 0 ? 1 : 0)*/) * frequency_d -
			current.time_since_epoch().count() };
    }
}