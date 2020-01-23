#pragma once
#include <cstddef>
#include <array>
#include <numeric>
#include <random>
#include <functional>

template <std::size_t N, typename T, std::size_t K, typename WEIGHT_TYPE, typename RND>
std::array<std::size_t, N> get_weighted_random_items(std::array<T, K>& array, std::function<WEIGHT_TYPE(T&)> func, RND& rnd)
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
	std::array<std::size_t, N> selected_indices{};
	std::size_t n = 0;
	for (auto i = 0; i < N; ++i)
	{
		
		//reroll:
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
