#pragma once
#include <chrono>
#include <queue>

namespace Utils
{
    using namespace std::chrono;
    using ms = time_point<milliseconds>;

    inline milliseconds get_time_left_until_next_period(const ms current, const milliseconds frequency)
    {
        //const double current_d{ static_cast<double>(current.time_since_epoch().count()) };
        const long long current_d{ current.time_since_epoch().count() };
        const long long frequency_d{ frequency.count() };
		const auto remainder = current_d % frequency_d;
		//const auto value = std::llround(frequency_d - remainder);
		const auto value = frequency_d - remainder;

		return milliseconds(value);
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


	template <typename T>
	class EventQueue : public std::priority_queue<T>
	{
	public:
		using iterator = typename std::priority_queue<T>::container_type::iterator;
		using const_iterator = typename std::priority_queue<T>::container_type::const_iterator;

		[[nodiscard]] iterator end() { return this->c.end(); }

		[[nodiscard]] const_iterator find(const T& val) const
		{
			auto begin = this->c.cbegin();
			auto end = this->c.cend();
			return std::find(begin, end, val);
		}

		[[nodiscard]] const_iterator find_by_id(std::size_t id) const
		{
			auto begin = this->c.cbegin();
			auto end = this->c.cend();
			while (begin != end)
			{
				if (begin->id == id)
					return begin;
				++begin;
			}
			return begin;
		}

		[[nodiscard]] iterator find_by_id(std::size_t id)
		{
			auto begin = this->c.begin();
			auto end = this->c.end();
			while (begin != end)
			{
				if (begin->id == id)
					return begin;
				++begin;
			}
			return begin;
		}

		void erase(const_iterator it)
		{
			this->c.erase(it);
			std::make_heap(this->c.begin(), this->c.end(), this->comp);
		}

		iterator add_delay(std::size_t id, milliseconds delay)
		{
			auto it = find_by_id(id);
			if (it == this->c.end())
				throw std::runtime_error("Can't find a item");
			
			auto copy = *it;
			erase(it);
			copy.time += delay;
			this->push(copy);
		}
	};
	
	class PoolAllocator
	{
		using T = milliseconds;
		const size_t per_page;
		const size_t pool_size = sizeof(T) * per_page;
		std::vector<T*> pools;
		size_t next_position;

		void alloc_pool()
		{
			next_position = 0;
			void* temp = operator new(pool_size);
			pools.push_back(static_cast<T*>(temp));
		}
	public:
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;

		PoolAllocator(size_t pool_size)
		: per_page{ pool_size }
		, pool_size{ pool_size * sizeof(T) }
		{
			alloc_pool();
		}

		T* operator()(T const &x)
		{
			if (next_position == per_page)
				alloc_pool();

			T* ret = new(pools.back() + next_position) T(x);
			++next_position;
			return ret;
		}

		~PoolAllocator()
		{
			while (!pools.empty())
			{
				T* p = pools.back();
				for (size_t pos = per_page; pos > 0; --pos)
					p[pos - 1].~T();
				operator delete(static_cast<void*>(p));
				pools.pop_back();
			}
		}
	};
}