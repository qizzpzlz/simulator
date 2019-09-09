#include <chrono>

namespace Utils
{
    using std::chrono::milliseconds;
    using ms = std::chrono::time_point<milliseconds>;

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
            using std::size_t;
            using std::hash;

            return hash<int64_t>{}(h.time_since_epoch().count());
        }
    };
}