#pragma once
#include <cstddef>
#include <array>
#include <numeric>
#include <random>
#include <functional>
#include <mutex>
#include <memory>
#include <iostream>

template <std::size_t N, typename T, std::size_t K, typename WEIGHT_TYPE, typename RND>
std::array<int, N> get_weighted_random_items(const std::array<T, K>& array, std::function<WEIGHT_TYPE(const T&)> func, RND& rnd)
{
	static_assert(K > 0);

	auto weights_ptr = std::make_unique<std::array<WEIGHT_TYPE, K>>();
	auto& weights = *weights_ptr;
	for (auto i = 0; i < K; ++i)
		weights[i] = func(array[i]);
	auto min_weight = *std::min_element(weights.begin(), weights.end());

	auto total_weight = static_cast<WEIGHT_TYPE>(0);
	for (auto i = 0; i < K; ++i)
	{
		weights[i] -= min_weight;
		total_weight += weights[i];
	}

	std::uniform_real_distribution<WEIGHT_TYPE> dist(0, total_weight);
	std::array<int, N> selected_indices{};
	selected_indices.fill(-1);
	std::size_t n = 0;

	std::size_t count = 0;

	for (int i = 0; i < N; ++i)
	{
		if (count++ > N * 10)
			std::cout << " ";

		auto roll = dist(rnd);
		for (auto j = 0; j < array.size(); ++j)
		{
			auto weight = weights[j];
			if (roll < weight)
			{
				bool reroll_flag{ false };
				for (auto k = 0; k < n; ++k)
				{
					if (selected_indices[k] == j)
					{
						reroll_flag = true;
						break;
					}
				}
				if (reroll_flag)
				{
					--i;
					break;
				}
				selected_indices[n++] = j;
				break;
			}
			else
			{
				roll -= weight;
			}
		}
	}

	return selected_indices;
}

template <std::size_t N, typename T, std::size_t K, typename RND>
std::array<std::size_t, N> get_random_items(const std::array<T, K>& array, RND& rnd)
{
	std::unique_ptr<std::array<std::size_t, K>> indices_ptr = std::make_unique<std::array<std::size_t, K>>();
	std::array<std::size_t, K>& indices = *indices_ptr;
	for (int i = 0; i < K; ++i)
		indices[i] = i;
	
	std::array<std::size_t, N> output;
	std::sample(indices.begin(), indices.end(), output.begin(), N, rnd);
	return output;
}

class _globalRandom
{
	void seed(int seed)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_engine.seed(seed);
	}
	double next_double()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _real_dist(_engine);
	}
	

	inline static std::mutex _mutex{};
	inline static std::mt19937_64 _engine{};
	inline static std::uniform_real_distribution<> _real_dist{};
};